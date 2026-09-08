# Mission 36 — Post-`c26d719` rangeproof cache key: still injective?

**Date:** 2026-09-08  
**Question centrale:** Après `c26d719` et #1592 / #1595 / #1599 / Sequentia #198, peut-on encore construire deux contextes rangeproof distincts qui partagent la même clé de cache et provoquer une réutilisation incorrecte d’une validation ?

**Verdict (catégorie demandée):** **B — COLLISION STRUCTURELLE TOUJOURS POSSIBLE**

Complément expérimental (hors catégorie seule) : au niveau `CachingRangeProofChecker`, le bypass lab Mission 28 **fonctionne encore** sur la formule actuelle → **ce n’est pas C**.  
`ConnectBlock` / acceptation bloc 4050336 : **NON DÉMONTRÉ** ici (comme Mission 28).

---

## 0. Chronologie (ne pas mélanger les couches)

| Couche | Faits |
|---|---|
| Code `c26d719` sur `master` | Merge #1592 **2026-09-01** |
| Contenu sur `23.3.x` | Cherry-pick `212c43f` **2026-09-03** (avant incident) |
| Merge PR #1599 | **2026-09-06 17:21 UTC** — **après** 13:53 UTC |
| Release `elements-23.3.3` | **pré-fix** |
| Build functionary le 6 | **UNKNOWN** |

`#1599` ≠ « le patch apparaît sur 23.3.x » : le contenu y était déjà le 3. `#1599` ne change **pas** la formule de clé (ci-dessous).

---

## 1. Formule exacte de la clé (FACT — git)

### Avant `c26d719`

```text
K = SHA256( salt_range_proof || proof || value_commitment )
```

Pas de length-prefix, pas de séparateur.

### `c26d719` et **tous** les états suivants vérifiés

Commits / refs dont la ligne `hasher.Write(proof)…` a été contrôlée :

- `c26d719c29` (fix)
- merge #1592 `31e8f27f4b`
- `6253d7e103` (23.x)
- `212c43f475` (23.3.x)
- merge #1599 `3b3f01eac9`
- `origin/elements-23.3.x` (HEAD local fetch)
- `origin/master` (HEAD)

```text
K = SHA256( salt_range_proof || proof || value_commitment || asset_commitment || scriptPubKey )
```

Implémentation (`sigcache.cpp`) :

```cpp
hasher.Write(proof.data(), proof.size())
      .Write(commitment.data(), commitment.size())
      .Write(asset_commitment.data(), asset_commitment.size())
      .Write(scriptPubKey.data(), scriptPubKey.size())
      .Finalize(entry.begin());
```

| Propriété | Présent ? |
|---|---|
| Length-prefix | **NON** |
| Séparateurs / tags | **NON** |
| Encodage canonique structuré | **NON** |
| Nouveaux champs depuis `c26d719` | **NON** |
| Ordre | `proof → value → asset → script` |

**Sequentia #198 :** même chaîne `Write` ; ajoute `rangeproof_cache_binding_test` seulement — **pas** de framing.

**Conclusion FACT :** #1592 / #1595 / #1599 / #198 **n’éliminent pas** l’ambiguïté de concaténation ; ils propagent ou testent le binding 4 champs **non cadré**.

---

## 2. Formalisation — injectivité de `serialize`

```text
K(x1…xn) = SHA256( salt || serialize(x1) || … || serialize(xn) )
serialize(xi) = octets bruts de xi   (identité)
```

Avec des longueurs variables **sans délimiteur**, `serialize` sur le *tuple* n’est **pas** injective :

```text
∃ (A,B) ≠ (A′,B′)  tq  A||B = A′||B′
```

Même en fixant `|value_commitment| = |asset_commitment| = 33` (chemin confidentiel Elements), il reste :

```text
P || C₃₃ || A₃₃ || S  =  P′ || C′₃₃ || A′₃₃ || S′
```

dès que des octets « glissent » entre `P` et `S` en emportant le bloc `C||A` (famille Mission 28).

**RÉSULTAT EXPÉRIMENTAL :** sur 2000 préimages aléatoires de longueur > 66 avec `|C|=|A|=33`, le taux de multi-parse (≥ 2 tuples distincts) = **1.0** (`final/data/36_cache_key_injectivity.json`).

Ce n’est **pas** une collision SHA-256 : c’est la **même préimage**.

---

## 3. Recheck Mission 28 sur la formule actuelle (= HEAD 23.3.x / master)

Fixtures : `final/hex/71c93d43_*` + `final/hex/out1_*`.

| Tuple | proof | value | asset (gen ser.) | script |
|---|---:|---:|---:|---:|
| **A (primer)** | P0 = 4166 | C0 = 33 | G = 33 | S0 = 69 |
| **B (attaque)** | P1 = 4234 | C1 = 33 | G = 33 | S1 = 1 (`6a`) |

Constructions (FACT byte-level) :

```text
P1 = P0 || C0 || G || 6a || 43
S0 = 6a || 43 || C1 || G || 6a
```

| Mesure | Résultat |
|---|---|
| Tuples logiquement distincts | **OUI** |
| Préimages unframed identiques | **OUI** (4301 bytes) |
| SHA-256(préimage) | `82b0b8cc…c01a` |
| Clés unframed égales (1000 salts) | **1000/1000** |
| Clés **framed** `len‖data` égales | **0/200** (même salt) |

### Harness natif `CachingRangeProofChecker` (replay)

`./build/28_cache_replay` → `final/logs/36_cache_replay_rerun.log` :

| Node | Mode | Résultat |
|---|---|---|
| A | POST-FIX, mempool Set primer puis Get attack | **RANGEPROOF_CHECKER ACCEPT** (hit cache, crypto non rejoué) |
| B | POST-FIX cold | **REJECT** (crypto attack = 0) |
| C | PRE-FIX primed | **REJECT** |
| D | POST-FIX + clé framed hypothétique | clés diffèrent → collision **cassée** |

**FACT :** la formule post-`c26d719` **actuelle** (identique sur 23.3.x HEAD) permet encore ce bypass lab.  
**NON DÉMONTRÉ :** acceptation full-node du bloc 4050336.

---

## 4. Fuzz / property search (au-delà de Mission 28)

### Famille généralisée « absorb C‖A into proof »

```text
t1 = (P,  C,  A,  mid || C2 || A2 || Ss)
t2 = (P||C||A||mid, C2, A2, Ss)
⇒ preimage(t1) = preimage(t2)  toujours
```

**RÉSULTAT :** 5/5 premiers essais aléatoires de la famille → collision unframed ; framed → **jamais** égale.

### Exhaustif 2 champs (alphabet `{a,b}`, len ≤ 3)

Collisions classiques du type `("", "a")` vs `("a", "")` — confirme non-injectivité de `||`.

### Random 10k tuples

Peu de collisions « accidentelles » hors famille structurée (attendu) ; la non-injectivité est **structurelle**, pas probabiliste.

---

## 5. Ce que prouve / ne prouve pas `rangeproof_cache_binding_test` (#198)

**Prouve :**

- asset et script entrent dans la clé ;
- même proof+value sous **autre** asset → pas de hit ;
- même proof+value+asset sous **autre** script → pas de hit ;
- proof d’issuance (script vide) ≠ replay sur script spendable **quand proof bytes identiques**.

**Ne prouve pas :**

- injectivité de la sérialisation ;
- absence d’alias où le **proof** absorbe `C||A||…` (Mission 28) ;
- que deux proofs *différents* ne peuvent pas partager une préimage.

**Réponse directe :** le test vérifie la *présence* de asset+script dans la clé pour des cas où proof‖value sont inchangés — **pas** la non-ambiguïté pour toutes les entrées à longueur variable.

Test complémentaire minimal (propriété) :

```text
∀ t1 ≠ t2,  framed_key(t1) ≠ framed_key(t2)
∧ ∃ t1 ≠ t2 (famille absorb), unframed_key(t1) = unframed_key(t2)
```

(déjà exécuté dans `36_cache_key_injectivity.json`)

---

## 6. Construction robuste (lab)

```text
framed:  SHA256( salt || len8(P)||P || len8(C)||C || len8(A)||A || len8(S)||S )
```

Sur Mission 28 et la famille absorb : **élimine** l’égalité de clés (EXPÉRIMENTAL).  
Autres options équivalentes : hash par champ `H(H(P)||H(C)||H(A)||H(S))`, tags CBOR/BIP, etc.

**Aucun de ces framings n’est dans Elements HEAD / 23.3.x / #198.**

---

## 7. Comparaison f24a / incident

| Point | Statut |
|---|---|
| Même *classe* (préimage unframed post-fix) toujours dans le code public | **FACT** |
| Paire primer `71c9` / sortie `f24a:1` = instance de cette classe | **FACT lab** (Mission 28 + recheck 36) |
| Functionaries tournaient ce code le 6 sep 13:53 | **UNKNOWN** |
| `#1599` @ 17:21 a « réparé » la clé | **FAUX** — formule inchangée ; merge post-incident seulement |
| Causalité historique 4050336 | **NON DÉMONTRÉ** |

Différence code 13:53 vs après #1599 sur la clé cache : **aucune** (même Write 4 champs unframed dès `212c43f` le 3).

---

## 8. Livrables

| # | Livrable | Emplacement |
|---|---|---|
| 1 | Diff clé `c26d719 → … → HEAD` | §1 (identique) |
| 2 | Formule actuelle | §1 |
| 3 | Contre-exemple d’injectivité | Mission 28 + famille §3–4 |
| 4 | Fuzz | `final/data/36_cache_key_injectivity.json` |
| 5–6 | Crypto + cache priming | `final/logs/36_cache_replay_rerun.log` |
| 7 | vs Mission 28 | recheck identique |
| 8 | Verdict | **B** |
| 9 | PoC local | `build/28_cache_replay` (non destructif) |
| 10 | Pourquoi « safe » est faux | framing absent ; #198 ne teste pas l’injectivité |

---

## 9. Classification épistémique

| Type | Contenu |
|---|---|
| **FACT** | Formule unframed 4 champs inchangée de `c26d719` à HEAD master/23.3.x ; #198 = tests only |
| **RÉSULTAT EXPÉRIMENTAL** | Préimages Mission 28 identiques ; 1000/1000 salts ; Node A ACCEPT ; framed casse la collision ; multi-parse 100 % |
| **INFERENCE** | Tout nœud exécutant ce code avec cache chaud primer peut court-circuiter la vérif crypto de l’alias (mécanisme) |
| **HYPOTHÈSE HISTORIQUE** | Ce mécanisme a accepté `f24a` en production — **non établie** |

---

## 10. Réponse en une phrase

**Oui — expérimentalement, après `c26d719` et après #1592/#1595/#1599/#198, on peut encore construire deux contextes distincts à même clé de cache (préimage identique non cadrée) et obtenir un ACCEPT du `CachingRangeProofChecker` après priming ; le correctif ajoute des champs mais ne rend pas la sérialisation injective.**
