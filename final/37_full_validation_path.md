# Mission 37 — De la collision du cache au chemin de validation complet

**Verdict : C**

**Nuance (obligatoire) :**  
**C** est établi expérimentalement au locus **`CheckTxInputs` → `VerifyAmounts` → `CRangeCheck` → `CachingRangeProofChecker`**, avec les sémantiques `cacheStore` de **mempool (`true`)** et de **`ConnectBlock` (`false`, consult only)**.  
Deux « nœuds » processus avec caches distincts **divergent** sur ce locus (ACCEPT vs REJECT) — comportement **de type D au niveau rangeproof**, mais **`elementsd` `ConnectBlock` / `ActivateBestChain` complet n’a pas été exécuté**.  
**E non revendiqué.** Causalité historique 4050336 : **NON DÉMONTRÉE**.

---

## Question finale

> La collision post-`c26d719` est-elle locale au checker, ou peut-elle influencer l’acceptation d’une transaction / d’un bloc dans le chemin de validation Elements ?

**Réponse expérimentale :** elle n’est **pas** locale au checker. Elle remonte jusqu’à l’appel que **`ConnectBlock` fait déjà** pour vérifier les montants (`CheckTxInputs` → `VerifyAmounts`). Sur ce locus, cache chaud → ACCEPT (crypto skip) ; cache froid → REJECT (crypto FALSE).  
**Limite falsifiable :** nous n’avons **pas** prouvé qu’un *bloc complet* passe tout `ConnectBlock` (scripts, surjection, UTXO, etc.) — seulement que **l’échec/succès de la vérif rangeproof consensus-path dépend de l’état du cache**.

---

## 1. Chemin de validation exact (FACT CODE)

Réf. locale : `elements` @ tip master / formule clé = post-`c26d719` (identique `23.3.x`).

```text
AcceptToMemoryPool / mempool validation
  → CheckInputScripts(..., cacheSigStore=true, ...)     [validation.cpp ~1493, ~449]
  → (amounts often via CheckTxInputs with cacheStore=true)

ConnectBlock(..., fJustCheck=false)                     [validation.cpp ~2780+]
  → for each non-coinbase tx:
      fCacheResults = fJustCheck;  // false when actually connecting
      Consensus::CheckTxInputs(..., cacheStore=fCacheResults, fScriptChecks, ...)
                                                    [validation.cpp ~3041–3045]
        → VerifyAmounts(spent_inputs, tx, pvChecks, cacheStore)
                                                    [consensus/tx_verify.cpp:250]
          → for confidential outs:
              QueueCheck(..., new CRangeCheck(..., store_result))
                                                    [confidential_validation.cpp:388]
                → CRangeCheck::operator()
                                                    [confidential_validation.cpp:61–70]
                  → CachingRangeProofChecker(store).VerifyRangeProof(...)
                                                    [script/sigcache.cpp:131–170]
                    → ComputeEntryRangeProof (unframed 4-field)
                    → Get(entry, !store)  // hit ⇒ return true, NO crypto
                    → else secp256k1_rangeproof_verify + min_value policy
                    → if ok && store: Set(entry)
```

**FACT CODE — sémantique cache dans `ConnectBlock` :**

```cpp
// validation.cpp ~3041
bool fCacheResults = fJustCheck; /* Don't cache results if we're actually connecting
                                    blocks (still consult the cache, though) */
```

| Phase | `store` / `cacheStore` | Get | Set |
|---|---|---|---|
| Mempool | `true` | consulte | **écrit** si crypto OK |
| ConnectBlock réel (`fJustCheck=false`) | `false` | **consulte** (erase mark) | **n’écrit pas** |

Le cache rangeproof est donc **process-global** (statique `rangeProofCache`), rempli surtout en **mempool**, consulté en **ConnectBlock**.

`ContextualCheckTransaction` : contrôles contextuels (locktime, etc.) — **pas** le locus rangeproof.  
Le locus montants confidential = **`CheckTxInputs` → `VerifyAmounts`**.

---

## 2. Instrumentation

Harness lab : `final/scripts/37_validation_path_harness.c`  
Binaire : `build/37_validation_path_harness`  
Log : `final/logs/37_full_validation.log`

Trace produite (extrait scénario C) :

```text
AcceptToMemoryPool
  → CheckTxInputs(cacheStore=1) → VerifyAmounts → CRangeCheck(primer)
     → Cache MISS → native = TRUE → Cache SET → ACCEPT

ConnectBlock(fJustCheck=false) primed
  → CheckTxInputs(cacheStore=0) → VerifyAmounts → CRangeCheck(alias)
     → Cache HIT → native SKIPPED → ACCEPT
```

---

## 3–4. Replay Mission 28 + validation transactionnelle

Fixtures inchangées : `71c9` out0 (primer) / `f24a` out1 (alias).

| Scénario | Description | ret | hit | crypto | Verdict locus |
|---|---|---:|---:|---:|---|
| **A** | Primer mempool `store=1` | 1 | 0 | 1 (TRUE) | ACCEPT + SET |
| **B** | Alias ConnectBlock cold `store=0` | 0 | 0 | 0 (FALSE) | **REJECT** (crypto) |
| **C** | Primer mempool → alias ConnectBlock | 1 | 1 | -1 (SKIP) | **ACCEPT** (bypass) |
| **D** | Primer puis alias en mempool `store=1` | 1 | 1 | -1 | **ACCEPT** (bypass) |

**Ce qui est contourné en C/D :** `secp256k1_rangeproof_verify` + politique `min_value==0`.  
**Ce qui échoue en B :** crypto native = FALSE.

---

## 5. `ConnectBlock` — portée expérimentale

| Couche | Statut |
|---|---|
| Appel `CheckTxInputs` depuis `ConnectBlock` avec `fCacheResults=false` | **FACT CODE** |
| Même sémantique rejouée dans le harness | **RÉSULTAT EXPÉRIMENTAL** |
| Bloc lab minimal assemblé + `elementsd` `ConnectBlock` | **NON FAIT** |
| `ActivateBestChain` / tip | **NON FAIT** |

**Pourquoi C quand même :** si `CheckTxInputs` échoue, `ConnectBlock` marque `BLOCK_CONSENSUS` et abandonne la tx/bloc (`validation.cpp` ~3046–3049). Donc l’état du cache **peut** décider du sort de cette vérification **sur le chemin réel de connexion de bloc**. Ce n’est plus « checker orphelin ».

---

## 6. Ordres testés

| Ordre | Résultat |
|---|---|
| primer → alias | alias **ACCEPT** (hit) |
| alias → primer | alias **REJECT** (crypto) ; primer ensuite OK — **pas de priming utile** |
| même « tx » séquentielle primer puis alias | alias **ACCEPT** (hit) |
| primer mempool → alias ConnectBlock | alias **ACCEPT** (hit) |

**Portée du cache (FACT + EXP) :** processus / instance `rangeProofCache` — traverse mempool → ConnectBlock ; pas limité à une seule transaction si le processus reste le même.

---

## 7. Deux nœuds (simulation processus)

| | Node A (primé) | Node B (froid) |
|---|---|---|
| ConnectBlock-locus alias | **ACCEPT** hit=1 crypto=SKIP | **REJECT** hit=0 crypto=FALSE |
| Divergence rangeproof locus | **OUI** | |

**INFERENCE :** deux nœuds stock avec mempools différents peuvent diverger sur **cette** vérification consensus.  
**NON DÉMONTRÉ :** divergence tip complète via `elementsd` P2P.

Le cache **est consensus-critical au sens suivant (FACT CODE + EXP locus) :** le résultat d’une vérif appelée depuis `ConnectBlock` dépend d’un état local non synchronisé (cache mempool), pas seulement des octets du bloc.

---

## 8. Comparaison 4050336 / `f24a`

| Question | Statut |
|---|---|
| Alias lab = champs `f24a:1` + construction Mission 28 | **FACT** (fixtures) |
| Primer on-chain `71c9` compatible byte-alias | **FACT lab** |
| `ConnectBlock(4050336)` via ce path | **NON DÉMONTRÉ** |
| Functionaries post-fix | **UNKNOWN** |

Messages PGP post-retour / `:(` : **piste contextuelle seulement** — **aucune** déduction de contenu ; **pas une preuve**.

---

## 9. Builds

| Build / formule clé | Comportement locus attendu |
|---|---|
| Pré-`c26d719` | Pas d’alias Mission 28 (clés 2 champs diffèrent) — Mission 28 Node C |
| `212c43f` / 23.3.x / HEAD / post-#1599 / #198 | **Même** formule unframed → **même** bypass locus |
| Framed lab | collision cassée → alias **REJECT** même après primer |

#1599 @ 17:21 ≠ changement de formule. Code « avant incident » sur 23.3.x (`212c43f` dès le 3) **≡** post-#1599 pour cette clé.

---

## 10. Autres caches unframed (scan `sigcache.cpp`)

| Fonction | Pattern | Action |
|---|---|---|
| `ComputeEntryRangeProof` | proof‖value‖asset‖script | **cette mission** |
| `ComputeEntrySurjectionProof` | wtxid32‖proof‖commitment | **candidat Mission séparée** |
| `ComputeEntryECDSA` / `Schnorr` | hash32‖key‖sig | longueurs variables ; threat model différent → **Mission séparée si poursuivi** |

---

## 11. Framing de référence (contrôle positif)

`len8‖field` × 4 → après primer, alias ConnectBlock : **REJECT** (MISS, crypto FALSE).  
Confirme que le bypass lab dépend du **manque de framing**.

---

## 12. Limites

1. Pas de `elementsd` ConnectBlock bout-en-bout.  
2. Pas d’autres checks (surjection, scripts, balance) dans le harness.  
3. Sel cache = constant lab (égalité relative des préimages suffit).  
4. Deux nœuds = deux `cache_t` processus, pas P2P.  
5. Historique Liquid / functionaries : **hors portée expérimentale**.

---

## 13. Classification

| Type | Contenu |
|---|---|
| **FACT CODE** | Chaîne ConnectBlock→CheckTxInputs→VerifyAmounts→CRangeCheck→checker ; `fCacheResults=fJustCheck` |
| **RÉSULTAT EXPÉRIMENTAL** | A–D ; ordres ; Node A/B divergence ; framed REJECT |
| **INFERENCE** | Sur un bloc où le seul échec cold est ce rangeproof, tip chaud/froid peut diverger |
| **HYPOTHÈSE HISTORIQUE** | 4050336 accepté ainsi — **non établie** |

---

## Artefacts

| Fichier | Rôle |
|---|---|
| `final/37_full_validation_path.md` | ce rapport |
| `final/data/37_validation_results.json` | résultats machine |
| `final/logs/37_full_validation.log` | trace harness |
| `final/scripts/37_validation_path_harness.c` | source |
| `build/37_validation_path_harness` | binaire |

---

**Une phrase :** la collision n’est pas confinée au checker — elle s’exerce sur le locus montants déjà invoqué par `ConnectBlock` ; deux états de cache peuvent y produire ACCEPT vs REJECT ; la preuve `elementsd` ConnectBlock complet et le lien historique 4050336 restent ouverts.
