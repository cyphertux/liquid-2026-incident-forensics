# 21 — Identifier le runtime qui a accepté 4050336

**Critère de succès :** Niveau 1 / 2 / 3, sinon Niveau 4.  
**Résultat de cette mission : NIVEAU 4.**

---

## EXECUTIVE RESULT

Aucun log RPC, debug.log, getblockchaininfo, version/commit/config de nœud de production, ni reproduction liée à un producteur identifiable n’a été trouvé pour `ConnectBlock(4050336)` / `ActivateBestChain → 4050336`.

Les signatures 4050336/4050337 et la chaîne parent sont reconfirmées. Le workflow stock permet de **produire** un bloc signé sans `TestBlockValidity` (`combineblocksigs`). Cela n’identifie pas le runtime d’**acceptation** chainstate.

**FIRST ACCEPTOR = UNKNOWN**  
**FIRST ACCEPTANCE MECHANISM = UNKNOWN**  
**FIRST UNPROVEN LINK = premier runtime ayant placé 4050336 dans un chainstate actif (ConnectBlock / équivalent)**

---

## MISSION SUCCESS LEVEL

| Niveau | Critère | Obtenu ? |
|---|---|---|
| 1 | Log/RPC nœud réel : 4050336 accepted / ActivateBestChain | **NON** |
| 2 | Runtime identifiable VERSION+COMMIT+CONFIG + ConnectBlock ACCEPT + lien producteur 4050337 | **NON** |
| 3 | ENV A REJECT vs ENV B ACCEPT, divergence reproductible, tous deux identifiés | **NON** |
| 4 | Aucune des trois | **OUI** |

---

## 1. PREUVES D’ENVIRONNEMENT CHERCHÉES

| Source | Résultat relatif à 4050336 / e1d9 / f24a / ConnectBlock |
|---|---|
| GitHub Issues/PR Elements (`4050336`, `e1d9`, rangeproof+inflation ciblés) | Aucun artefact d’acceptation de ce bloc |
| Releases Elements | `elements-23.3.3` publié 2026-04-13 — **CODE EXISTS**, pas **DEPLOYED on acceptor** |
| Blog opérateur (weezey, juin 2026) | Un nœud public à `v23.3.3` bloqué à hauteur ~3.9M — **pas** le tip 4050336, **pas** un functionary |
| Médias / SideSwap / Blockstream X (2026-09-06) | Peg-out / « bug Elements » — **pas** version, config, ni log ConnectBlock |
| Discord/Telegram/Reddit/captures debug.log | **Aucun** trouvé cette mission |
| Explorer APIs | Blockstream : hash unique à 4050336 = `e1d9…` ; `next_best` = `c212…` ; mempool.space HTML shell (pas d’API tip alternative récupérée) |
| Docker/manifests functionary | **Aucun** pin version pour le 2026-09-06 |

**Aucune preuve Niveau 1–3.**

---

## 2. PRODUCTEUR DE 4050337

| Champ | Valeur |
|---|---|
| BLOCK | 4050337 / `c212cdcb6b2e68d4…38be` |
| PREV | `e1d9a2aa…a0d5` = hash(4050336) — **CONFIRMED** |
| PUBKEYS | 15 pubs fédération (`final/data/block_4050336_fed_pubkeys.txt`) |
| SIGNATURE POSITIONS | `0,1,3,5,6,7,8,10,11,13,14` — **CONFIRMED** ECDSA |
| COMMON SIGNERS vs 4050336 | intersection `{0,1,3,5,6,7,8,11,13,14}` |
| POSSIBLE PRODUCER | **UNKNOWN** (round-robin documenté ; pas de mapping pub→opérateur→slot dans les données publiques utilisées) |
| EVIDENCE | header + witness seulement |
| CONFIDENCE | producteur logique : **UNKNOWN** |

Docs Blockstream (FAQ functionary, màj 2026-08-06) : round-robin de proposition ; HSM suit hauteur + prevhash du **dernier bloc signé**.  
Cela contraint la **politique de signature HSM**, **pas** l’exécution de `ConnectBlock`.

**Signer 4050337 ≠ preuve d’avoir accepté 4050336 dans le chainstate.**  
**POSSIBLE PRODUCER n’est pas promu FIRST ACCEPTOR.**

---

## 3. SIGNATURES 4050336 / 4050337 (recalcul indépendant)

Méthode : hash bloc LE + DER + `SIGHASH_ALL` ; matching CHECKMULTISIG.

| BLOCK | POSITIONS | |
|---:|---|---|
| 4050336 | 0,1,3,4,5,6,7,8,11,13,14 | |
| 4050337 | 0,1,3,5,6,7,8,10,11,13,14 | |

| Diff | Positions |
|---|---|
| Seulement 4050336 | `{4}` |
| Seulement 4050337 | `{10}` |
| Intersection | 10 clés |
| Commun à 4050335–4050343 (mission 20) | `{1,5,6,8,11}` |

**Autorisé :** « les cinq clés `{1,5,6,8,11}` ont signé tous les blocs observés de la série ».  
**Interdit / non conclu :** « ces cinq clés ont validé 4050336 via ConnectBlock ».

Quorum 4050337 ≠ 4050336 (permutation / substitution 4↔10).

---

## 4. WORKFLOW PRODUCTEUR (code Elements public)

| FUNCTION | CALLER | VALIDATION AMOUNTS? | CONDITIONS | SOURCE | CONFIDENCE |
|---|---|---|---|---|---|
| `getnewblockhex` → `CreateNewBlock` | RPC mining | **OUI** si `test_block_validity` | défaut `true` (`miner.h:182`) | `rpc/mining.cpp`, `node/miner.cpp` | HIGH (code) |
| `TestBlockValidity` | CreateNewBlock / wallet | **OUI** (ConnectBlock fJustCheck) | — | validation | HIGH |
| wallet `signblock` | RPC wallet | **OUI** avant signe | docstring QA/testing | `wallet/rpc/elements.cpp:59–108` | HIGH |
| `combineblocksigs` | RPC mining | **NON** | assemble + `CheckProof` only | `rpc/mining.cpp:1297–1380` | HIGH |
| `ProcessNewBlock` / `AcceptBlock` | P2P/RPC | structure / stockage | avant tip | validation | HIGH |
| `ActivateBestChain` → `ConnectBlock` | chainstate | **OUI** si `fScriptChecks` | assumevalid gate | `validation.cpp` | HIGH |

**Existe-t-il un chemin officiel pour produire un bloc signé sans `TestBlockValidity` ?**  
**OUI → PRODUCTION OF INVALID BLOCK = CONFIRMED (code path `combineblocksigs` / hex injecté).**  

**≠ ACCEPTANCE OF INVALID BLOCK.**

---

## 5. PREMIER BLOC QUI FORCE L’ACCEPTATION ?

| Bloc | Implication stricte |
|---|---|
| 4050336 | Signé + contenu f24a — **pas** preuve chainstate d’un nœud nommé |
| 4050337 | Construit/signé/propagé avec `prev=e1d9` — prouve qu’**un** environnement a choisi ce parent ; **n’identifie pas** lequel avait ConnectBlock’d 4050336 |
| 4050338+ | Même branche ; n’ajoutent pas d’identité de runtime |

Spend `f24a:0` en 4050337 (mission 19) : **INFERENCE** qu’un tip UTXO avait connecté 4050336 — toujours **sans** nommer le nœud.

---

## 6. LOGS D’ACCEPTATION

| TIMESTAMP | ACTOR | EVENT | BLOCK | RESULT | SOURCE | CONFIDENCE |
|---|---|---|---|---|---|---|
| — | — | ConnectBlock / ActivateBestChain / rejected | 4050336 | — | recherche publique + workspace | **UNKNOWN** (aucun) |
| 1788702790 | index Blockstream | block indexed | 4050336 | visible | API status JSON | FACT affichage, ≠ ConnectBlock nommé |
| 1788702850 | index Blockstream | next block | 4050337 | visible | API | FACT |

**Aucune preuve FORTE du type « ConnectBlock(4050336) succeeded » sur un nœud identifiable.**

---

## 7. PREUVE CHAINSTATE (RPC)

| Artefact | Trouvé ? |
|---|---|
| `getblockchaininfo` / `blocks=4050336` d’un nœud nommé | **NON** |
| `getbestblockhash` = e1d9 puis c212 | **NON** (hors API explorer) |
| debug.log ProcessNewBlock | **NON** |

**NODE / VERSION / COMMIT / CONFIG pour bestblock 4050336→4050337 : UNKNOWN**

---

## 8. PRODUCTEUR vs VALIDATEUR

| NODE | 4050336 | 4050337 |
|---|---|---|
| A (producteur template) | ACCEPT/REJECT/**UNKNOWN** | PRODUCED/**UNKNOWN** qui |
| B (signataires 11/15) | **UNKNOWN** chacun | signed (positions connues) |
| C (peers / tip builders) | **UNKNOWN** | **UNKNOWN** |
| Blockstream explorer backend | **UNKNOWN** mécanisme | affiche la chaîne |

Pas de matrice ACCEPT/REJECT empirique entre deux nœuds identifiés.

---

## 9. VERSION / BUILD FORENSICS

| VERSION | CODE EXISTS | CODE DEPLOYED (acceptor) | CODE ACTIVE AT 4050336 |
|---|---|---|---|
| elements-23.3.3 | FACT (release) | UNKNOWN | UNKNOWN |
| cache-key fix `c26d719` (2026-09-01) | FACT (git) | UNKNOWN | UNKNOWN |
| Liquid-specific private patches | UNKNOWN (non trouvés publiquement ici) | UNKNOWN | UNKNOWN |
| Functionary HSM firmware | UNKNOWN | UNKNOWN | UNKNOWN |

Différences code affectant VerifyAmounts / fScriptChecks / cache : **étudiées missions antérieures** ; **aucune** liée à un déploiement prouvé sur l’accepteur.

**CODE DIFFERENCE Liquid≠upstream expliquant ACCEPT : non établie cette mission.**  
Si un patch privé existait : `CODE DIFFERENCE` serait FACT seulement après publication ; `CAUSALITY` resterait à reproduire.

---

## 10. ENVIRONNEMENTS TESTABLES IDENTIFIÉS

**Aucun environnement de production documenté** (version+config) pour rejouer Accept.

| ENV | STATUS |
|---|---|
| Lab stock Elements (missions 15–18) | REJECT f24a, cache vide, fScriptChecks=true |
| Lab fScriptChecks=false / assumevalid | ACCEPT possible **en code** — **pas** un env prod |
| Runtime prod tip | **non identifié → non testé** |

---

## 11. PATCHES LIQUID SPÉCIFIQUES

Recherche publique : pas de commit/patch post-incident publiant un validateur divergant pour 4050336.  
**stock Elements = REJECT** (lab) vs **Liquid production = ACCEPT** : écart **observé au niveau chaîne**, mécanisme runtime **UNKNOWN**.

---

## 12. fScriptChecks=false

```
AssumedValidBlock non-null
  + pindex ancêtre de assumevalid et de best_header
  + equivalent-time ≤ 2 semaines
  ↓
fScriptChecks = false   (validation.cpp ~2851–2877)
  ↓
VerifyAmounts skipped   (tx_verify.cpp:250)
  ↓
f24a peut passer ConnectBlock sans RP
```

Autres assignations ConnectBlock : **aucune** (seul `= true` puis éventuellement `= false` via cette porte).  
Mempool a son propre `fScriptChecks=true` local.

Liquid v1 `defaultAssumeValid = uint256()` (`chainparams.cpp` CLiquidV1Params).

**Condition existait-elle sur un nœud Liquid prod le 2026-09-06 ?** → **UNKNOWN**  
**fScriptChecks SCENARIO = THEORETICAL ONLY.**

---

## 13. FLAGS BLOCK INDEX

| Flag | Définition | Skip VerifyAmounts ? |
|---|---|---|
| BLOCK_VALID_HEADER / TREE / TRANSACTIONS / CHAIN | étapes avant scripts | NON pour amounts au Connect |
| BLOCK_VALID_SCRIPTS | `RaiseValidity` **après** ConnectBlock réussi (~3172) | N’autorise pas un skip a priori : marque le succès |

Persistance disk de `nStatus` : oui (block index).  
Un nœud ne peut pas « recevoir 4050336 déjà SCRIPTS-valid » sans l’avoir connecté (ou restauré un index issu d’un Connect réussi antérieur sur **cette** machine) — **pas de preuve** d’un tel état pour un nœud nommé.

**Distinguer :** rangeproof cache ≠ block index `nStatus` ≠ coins chainstate.

---

## 14. REINDEX / RESTART / SNAPSHOT

Aucun chemin de code trouvé cette mission où reindex/restart/pruning/assumeutxo **accepte** une RP invalide sans passer VerifyAmounts (sauf assumevalid / cache déjà documentés).  
**Sans preuve prod :** pas d’explication reindex pour 4050336.

---

## 15. BLOCKSTREAM

| Question | Réponse |
|---|---|
| « Affiche 4050336 » | API : `in_best_chain:true`, height 4050336, `next_best=c212…` |
| Backend / version / validation | **BLOCKSTREAM ACCEPTANCE MECHANISM = UNKNOWN** |
| Preuve forte indépendante | `4050337.prev = hash(4050336)` (+ suite) |

---

## 16. CHAÎNE CONCURRENTE

| Check | Résultat |
|---|---|
| `block-height/4050336` Blockstream | unique `e1d9…` |
| Autre hash même prev(4050335) | **non trouvé** dans les sources accessibles |
| CONSENSUS SPLIT | **UNKNOWN** (pas CONFIRMED ; une API ≠ preuve d’absence globale) |

---

## 17. FALSIFICATION CACHE (sans réhabiliter)

| Question | Réponse |
|---|---|
| Acceptable **sans** cache hit (code) ? | **OUI** (`fScriptChecks=false`) → **CACHE NOT NECESSARY** comme unique mécanisme |
| Cache hit démontré sur env réel ? | **NON** → CACHE PRESENT = **UNKNOWN** |
| Priming P1/C1 historique ? | **UNKNOWN** / non démontré |
| Cache causal pour l’incident ? | **non démontré** |

---

## 18. DEUX GRAPHES

### A — CRYPTO
```
f24a → P1 → rangeproof_verify FAIL → stock ConnectBlock REJECT
```

### B — CONSENSUS
```
4050335 → 4050336 → 4050337 → … → 4050343
```

### Lien
```
P1 invalid
    │
    ▼
???  [aucune preuve de mécanisme runtime]
    │
    ▼
4050336 in some active chainstate
    │
    ▼
4050337.prev = e1d9
```

---

## 20. VERDICT A–N

| # | Question | Answer | Status |
|---|---|---|---|
| A | 4050336 cryptographically linked to 4050337? | YES | **CONFIRMED** |
| B | 4050337 producer identified? | NO | **UNKNOWN** |
| C | First node known? | NO | **UNKNOWN** |
| D | First runtime known? | NO | **UNKNOWN** |
| E | Version known? | NO | **UNKNOWN** |
| F | Config known? | NO | **UNKNOWN** |
| G | ConnectBlock(4050336) acceptance demonstrated? | NO | **UNKNOWN** |
| H | Another node rejection demonstrated? | NO | **UNKNOWN** |
| I | Consensus split demonstrated? | NO | **UNKNOWN** |
| J | Cache hit demonstrated? | NO | **UNKNOWN** |
| K | Cache necessary? | No as sole code path | **REFUTED** as necessary; hist role **UNKNOWN** |
| L | Cache sufficient historically? | Lab yes; hist no | lab PLAUSIBLE / hist **UNKNOWN** |
| M | f24a causal mechanism demonstrated? | Content of signed block; tip path not mechanistically closed | presence **CONFIRMED**; mechanism **UNKNOWN** |
| N | First unknown causal link? | Runtime placing 4050336 in active chainstate | **UNKNOWN** link |

---

## 21. TABLEAU FINAL

| CLAIM | STATUS | EVIDENCE | CONFIDENCE |
|---|---|---|---|
| 4050337 descends from 4050336 | CONFIRMED | prevhash == e1d9 | HIGH |
| 4050336 signed 11/15 | CONFIRMED | ECDSA positions | HIGH |
| f24a rangeproof invalid | CONFIRMED | independent verify | HIGH |
| stock ConnectBlock rejects | CONFIRMED | code + lab | HIGH |
| runtime X accepted | UNKNOWN | aucun artefact | — |
| runtime X produced 4050337 | UNKNOWN | — | — |
| runtime X version | UNKNOWN | — | — |
| runtime X configuration | UNKNOWN | — | — |
| other node rejected | UNKNOWN | — | — |
| consensus split | UNKNOWN | no dual tip found; not proven absent | — |
| cache hit | UNKNOWN | — | — |
| cache causal | UNKNOWN / not demonstrated | — | — |
| f24a in signed block causal to tip content | CONFIRMED (merkle/auth) | block bytes | HIGH |
| f24a explains why ConnectBlock passed | UNKNOWN | — | — |
| combineblocksigs can skip TestBlockValidity | CONFIRMED | mining.cpp | HIGH |
| production acceptance path | UNKNOWN | — | — |

---

## 22. CONCLUSION STRICTE

**FIRST ACCEPTOR:**  
UNKNOWN

**FIRST ACCEPTANCE MECHANISM:**  
UNKNOWN

**FIRST UNPROVEN LINK:**  
Le premier runtime (node/version/config) ayant placé le bloc `e1d9…` (4050336) dans un **chainstate actif** (succès `ConnectBlock` / `ActivateBestChain` ou équivalent prouvé), avant ou pour permettre `4050337.prev = e1d9…`.

**NO CAUSAL NARRATIVE IS JUSTIFIED BEYOND THIS POINT.**

---

## Artefacts

- `final/data/signblock_pubkey_matches.json`
- `final/data/chain_table_4050335_4050343.json`
- `final/data/block_4050336_status.json`
- `final/20_validation_path_ancestor.md` (chaîne / scores antérieurs)
