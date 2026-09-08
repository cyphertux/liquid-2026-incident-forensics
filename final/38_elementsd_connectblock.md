# Mission 38 — C → D : le cache peut-il influencer `elementsd` `ConnectBlock` ?

**Verdict : C** (inchangé)

**D déclaré : NON**  
**E déclaré : NON**

### Question centrale

> Deux instances identiques d’`elementsd`, recevant le même bloc, peuvent-elles diverger **uniquement** parce que l’une a dans son cache une validation antérieure correspondant à la collision structurelle Mission 28 ?

**Réponse :** **NON DÉMONTRÉ.**  
Les critères obligatoires pour D ne sont pas remplis. La barrière principale n’est pas une réfutation du mécanisme : c’est l’**absence d’un binaire post-`c26d719` exécutable** dans cet environnement, puis l’**injection** des fixtures Liquid dans un vrai `ProcessNewBlock`/`ConnectBlock`.

Mission 37 reste le plafond expérimental actuel (**C** au locus `CheckTxInputs` → … → checker).

---

## 1. Commit / build exact

| Élément | Valeur |
|---|---|
| Code source étudié | `ElementsProject/elements` @ `c7e856fab1b0c4d37005e25c0940184d812a26a0` (master, **post-fix** unframed) |
| Équivalent 23.3.x étudié auparavant | `212c43f475` / tip post-#1599 — **même formule de clé** |
| `elementsd` exécuté en lab | **v23.3.3** `aarch64-linux-gnu` (release GitHub) |
| Post-fix dans ce binaire ? | **NON** — `elements-23.3.3` est **pré-`c26d719`** (Mission 34) |
| Compilation locale post-fix | **BLOQUÉE** : pas de `cmake`/`autoconf` ; pas de `sudo` pour deps (comme Mission 06) |
| Modifications locales du consensus | **aucune** (pas d’instrumentation compilée dans un binaire post-fix) |

Le binaire 23.3.3 sert uniquement de **contrôle d’infrastructure** (démarrage, `InitRangeproofCache`), **pas** de test du bypass Mission 28.

---

## 2. Architecture du cache (FACT CODE + log `elementsd`)

### Code (`sigcache.cpp`)

```cpp
// TODO: de-globalise
namespace {
    static SignatureCache rangeProofCache;      // process-global
    static SignatureCache surjectionProofCache;
}
```

| Propriété | Faits |
|---|---|
| Instance | **statique globale** (pas par bloc / pas par tx) |
| Init | `InitRangeproofCache` une fois (`init.cpp` / AppInit) |
| Utilisateur | `CachingRangeProofChecker` → **cette** `rangeProofCache` |
| Partage mempool ↔ ConnectBlock | **OUI** (même process) |
| Persistance disque | **NON** (RAM cuckoo) |
| ConnectBlock | `fCacheResults = fJustCheck` → en connexion réelle **`store=false`** : consulte, **n’écrit pas** |
| `erase=true` | `contains(erase)` marque reclaimable ; entrée **encore trouvable** jusqu’éviction (Mission 28) |

### Log réel `elementsd` v23.3.3 (regtest Node A)

```text
Using 8 MiB out of 8/4 requested for rangeproof cache, able to store 262144 elements
Using 8 MiB out of 8/4 requested for surjectionproof cache, able to store 262144 elements
```

**FACT :** le nœud réel initialise bien le cache rangeproof.  
**FACT :** ce n’est **pas** une nouvelle instance par `ConnectBlock` — le commentaire source « TODO: de-globalise » + usage static le prouvent.  
**Alignement Mission 37 :** le harness simulait la bonne sémantique `store` ; l’instance réelle est bien process-global.

---

## 3. Chemin réel de validation (rappel FACT CODE)

Identique Mission 37 — inchangé dans le source post-fix :

```text
ProcessNewBlock / ActivateBestChain
  → ConnectBlock (fJustCheck=false)
      → CheckTxInputs(..., cacheStore=false, ...)
          → VerifyAmounts(..., store_result=false)
              → CRangeCheck → CachingRangeProofChecker(store=false)
                  → rangeProofCache.Get (consult)
```

Mempool : `store=true` → `Set` possible.

---

## 4–11. Tests 1–8 / framed / multi-node post-fix

| Test | Statut | Motif |
|---|---|---|
| 1 Alias seul (post-fix `elementsd`) | **NON EXÉCUTÉ** | Pas de binaire post-fix |
| 2 Primer → alias | **NON EXÉCUTÉ** | idem |
| 3 Vrai `ConnectBlock` / `ProcessNewBlock` | **NON EXÉCUTÉ** | idem + pas de bloc lab avec fixtures |
| 4 Deux `elementsd` post-fix | **NON EXÉCUTÉ** | idem |
| 5 Ordres tx | **NON EXÉCUTÉ** sur nœud ; **fait** au locus Mission 37 |
| 6 Restart | **INFERENCE CODE** : cache RAM → restart purge ; **non mesuré** post-fix |
| 7 Erase/éviction | **CODE** seulement (cuckoocache) |
| 8 Framed control | **Mission 37 harness** seulement |
| 9 `f24a` injecté | **NON EXÉCUTÉ** | fragments ≠ tx/bloc valides |
| 10 Contexte 4050336 | **NON EXÉCUTÉ** | historique + build functionary UNKNOWN |

### Contrôle infrastructure (pré-fix)

- Téléchargement `elements-23.3.3-aarch64-linux-gnu`
- Démarrage regtest Node A → RPC `getblockchaininfo` OK
- Log `InitRangeproofCache` capturé  
→ prouve qu’on *peut* faire tourner `elementsd` ici, **pas** qu’on a testé D.

---

## 12. Checklist D (aucune case post-fix cochée)

| Critère | Statut |
|---|---|
| même build **post-fix** | ✗ |
| même configuration | N/A |
| même bloc | N/A |
| même UTXO | N/A |
| Node A cache chaud | N/A |
| Node B cache froid | N/A |
| même chemin ConnectBlock | code ✓ / runtime post-fix ✗ |
| différence uniquement CACHE HIT/MISS | N/A |
| A accepte / B rejette | N/A |
| aucune validation ultérieure n’égalise | N/A |
| reproductible | ✗ |

**Donc D non déclaré** (règle Mission 38 respectée).

---

## 13. Point exact où la chaîne s’arrête

```text
Mission 36–37 harness (C)
        ↓
CachingRangeProofChecker .......... EXP OK
CheckTxInputs / VerifyAmounts ..... EXP OK (simulé store flags)
ConnectBlock sémantique store ..... FACT CODE + EXP locus
        ↓
elementsd binaire POST-FIX ........ ✗ BARRIÈRE
        ↓
Bloc lab avec primer/alias ........ ✗ BARRIÈRE
ProcessNewBlock / ConnectBlock .... ✗ non atteint
Tip / multi-node divergence ....... ✗ non atteint
4050336 historique ................ ✗ non atteint (E)
```

**Première barrière fatale pour D :** pas d’`elementsd` post-`c26d719` exécutable.  
**Deuxième barrière :** même avec un tel binaire, il faudrait reconstruire des **transactions confidentielles valides** portant les octets Mission 28 (UTXO, scripts, surjection, fees) — non fait.

Ce n’est **pas** une preuve que D est faux ; c’est une preuve que **D n’est pas établi ici**.

---

## 14. Classification épistémique

| Type | Contenu |
|---|---|
| **FACT CODE** | `rangeProofCache` static ; ConnectBlock `store=false` ; chemin VerifyAmounts |
| **RÉSULTAT EXPÉRIMENTAL** | `elementsd` 23.3.3 démarre ; log init cache ; Mission 37 inchangée |
| **INFERENCE** | Un binaire post-fix *devrait* partager le même cache process-global (code) |
| **HYPOTHÈSE** | Divergence tip A/B sur un vrai bloc — **non testée** |

---

## 15. Ce qu’il faudrait pour prétendre D (prochaine lab)

1. Build ou binaire **post-`c26d719`** (ex. tip `23.3.x` / `master` compilé).  
2. Instrumentation `CACHE HIT/MISS` dans `VerifyRangeProof`.  
3. Chaîne regtest + txs valides portant primer/alias (ou reconstruction fidèle).  
4. Deux datadirs, même conf, même `block hex`.  
5. Logs prouvant HIT vs MISS comme seule divergence.  
6. Vérifier qu’aucune check ultérieure n’égalise les verdicts.

Sans cela → rester à **C**.

---

## Artefacts

| Fichier | Contenu |
|---|---|
| `final/38_elementsd_connectblock.md` | ce rapport |
| `final/data/38_elementsd_results.json` | résultats machine |
| `final/logs/38_elementsd_full.log` | version + log cache init + barrières |
| `final/data/elementsd_lab/` | lab local (**binaires gitignorés**, ne pas pusher) |

---

## Une phrase

**Le mécanisme reste compatible avec une divergence consensus-path (C + architecture cache process-globale), mais Mission 38 n’a pas exécuté le vrai `ConnectBlock` post-fix sur deux `elementsd` — donc on ne passe pas à D.**
