# 20 — Chemin de validation ayant permis à 4050336 de devenir ancêtre

**SIGNATURE ≠ VALIDATION**  
**DESCENDANT BLOCK ≠ PREUVE DE LA VALIDATION PAR CHAQUE SIGNATAIRE**

---

## 1. EXECUTIVE RESULT

**FIRST ACCEPTOR = UNKNOWN.**

La chaîne parent cryptographiquement vérifiée `4050335 → … → 4050343` prouve que `4050337.prev == hash(4050336)` et que la branche a continué sous quorum 11-of-15. Les 11 signatures de chaque bloc ont été associées aux positions de clés fédération (ECDSA sur hash de bloc LE + SIGHASH_ALL). Stock Elements avec cache vide et `fScriptChecks=true` rejette f24a dans `VerifyAmounts`. Aucune preuve de production n’identifie version, config (`assumevalid` / cache), ni le premier runtime ayant réussi `ConnectBlock(4050336)`. Le premier écart observable reste : **attendu REJECT vs ancêtre réel** — le « ? » entre signature et tip avancé n’est pas localisé.

---

## 2. BLOCK CHAIN PROOF

Source : API Blockstream Liquid + JSON locaux `final/data/block_*.json`.  
Vérification : `prev(h) == id(h-1)` pour h∈[4050336,4050343] → **CHAIN_OK = TRUE**.

| HEIGHT | HASH | PREVHASH | SIGNATURES | SIGNER POS | NTX | TIMESTAMP | SOURCE |
|---:|---|---|---:|---|---:|---:|---|
| 4050335 | `aad24e4fb64ca8adf4961667da87820cd48e553957ac64e75de7cdb298b5d66b` | `3b892680a538bdc4cef5b735176e541d79851cd1834fb32a071d86d01609ffe0` | 11 | 0,1,3,4,5,6,7,8,10,11,13 | 5 | 1788702730 | blockstream API + ECDSA |
| 4050336 | `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5` | `aad24e4fb64ca8adf4961667da87820cd48e553957ac64e75de7cdb298b5d66b` | 11 | 0,1,3,4,5,6,7,8,11,13,14 | 7 | 1788702790 | idem |
| 4050337 | `c212cdcb6b2e68d4f56a7ddfee48bd3c02b2bfbb703b9f4506ac1597a40d38be` | `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5` | 11 | 0,1,3,5,6,7,8,10,11,13,14 | 6 | 1788702850 | idem |
| 4050338 | `0b998712cb557e04f1780e6285509fd3c6ed94344bdeb9ad9c8f819bfd232250` | `c212cdcb6b2e68d4f56a7ddfee48bd3c02b2bfbb703b9f4506ac1597a40d38be` | 11 | 1,2,3,4,5,6,7,8,10,11,13 | 4 | 1788702910 | idem |
| 4050339 | `5ba1ea5defeebca7c4036c9242461be8cfd7e6de055156972c1c87d5b58e52cb` | `0b998712cb557e04f1780e6285509fd3c6ed94344bdeb9ad9c8f819bfd232250` | 11 | 0,1,3,4,5,6,8,10,11,13,14 | 3 | 1788702970 | idem |
| 4050340 | `4a308cc2b35e675a8df9cd04e1748625c8200dd169ff40b522b2e10f2ee5526d` | `5ba1ea5defeebca7c4036c9242461be8cfd7e6de055156972c1c87d5b58e52cb` | 11 | 0,1,3,4,5,6,7,8,10,11,14 | 2 | 1788703030 | idem |
| 4050341 | `37a4edbe432dd6bb676aa8657e01a7aaaeab1c01e7fc267d02c1dc025f7147fb` | `4a308cc2b35e675a8df9cd04e1748625c8200dd169ff40b522b2e10f2ee5526d` | 11 | 0,1,4,5,6,7,8,10,11,13,14 | 4 | 1788703090 | idem |
| 4050342 | `90d1508b879a8f5ed832615785d653b0f2a6c06b485f1950665e93ea13e97aed` | `37a4edbe432dd6bb676aa8657e01a7aaaeab1c01e7fc267d02c1dc025f7147fb` | 11 | 1,3,4,5,6,7,8,10,11,12,13 | 2 | 1788703150 | idem |
| 4050343 | `f7add9732519fd2737a1be40a86a96191db0ea126338cc934eb686834ddc98f8` | `90d1508b879a8f5ed832615785d653b0f2a6c06b485f1950665e93ea13e97aed` | 11 | 0,1,4,5,6,7,8,9,10,11,13 | 6 | 1788703210 | idem |

`signblockscript` identique (P2WSH `00207f1a37f6…e6be74`) sur toute la série.  
Δt 4050336→4050337 = **60 s**.

Artefacts : `final/data/chain_table_4050335_4050343.json`, `final/data/signblock_pubkey_matches.json`.

---

## 3. SIGNATURE ANALYSIS

Méthode : `GenericVerifyScript` → `SimpleSignatureChecker(block.GetHash(), sighash_byte=true)` ; message = hash bloc **ordre interne LE** (= `bytes.fromhex(id)[::-1]`) ; DER + `SIGHASH_ALL` ; matching CHECKMULTISIG (parcours pubs dans l’ordre).

| BLOCK | PUBKEY POSITIONS WITH SIGNATURE PRESENT | CONFIDENCE |
|---:|---|---|
| 4050336 | 0,1,3,4,5,6,7,8,11,13,14 | CONFIRMED (ECDSA) |
| 4050337 | 0,1,3,5,6,7,8,10,11,13,14 | CONFIRMED |
| 4050338 | 1,2,3,4,5,6,7,8,10,11,13 | CONFIRMED |
| … | voir table §2 | CONFIRMED |

Pubkeys (`final/data/block_4050336_fed_pubkeys.txt`) — pas d’identité humaine :

| POS | PUBKEY |
|---:|---|
| 0 | `026a2a106ec32c8a1e8052e5d02a7b0a150423dbd9b116fc48d46630ff6e6a05b9` |
| 1 | `03326b356f7ad58556815a3ab2e606789bc5b5d10c2f38ebc7fb4c5692a0d3cfa0` |
| 3 | `02aee8967150dee220f613de3b239320355a498808084a93eaf39a34dcd6202485` |
| … | (15 clés ; positions 2,9,12 absentes de 4050336) |

- **Intersection 4050336 ∩ 4050337** : `{0,1,3,5,6,7,8,11,13,14}` (10 clés)  
- **Commun à tous les blocs 4050335–4050343** : `{1,5,6,8,11}`  
- Une signature prouve l’autorisation du **challenge signblock**, **pas** l’exécution de `TestBlockValidity` / `ConnectBlock`.

---

## 4. PRODUCER ANALYSIS

| Question | Verdict | Label |
|---|---|---|
| Constructeur de 4050336 | UNKNOWN | UNKNOWN |
| Producteur (template) de 4050337 | UNKNOWN | UNKNOWN |
| Le producteur de 4050337 = premier accepteur de 4050336 ? | UNKNOWN | UNKNOWN |

Classement producteur 4050337 comme premier accepteur :

| Claim | Rank |
|---|---|
| A produit le header avec `prev=e1d9` | **CONFIRMED** (contenu du bloc) |
| A a forcément exécuté `ConnectBlock(4050336)` avec succès | **UNKNOWN** (pas de log) |
| Environnement A = premier accepteur global | **UNKNOWN** |

**PRODUCTEUR ≠ VALIDATEUR (code stock) — CONFIRMED possible :**

| Étape | Qui (stock) | Vérifie amounts ? |
|---|---|---|
| Template | `CreateNewBlock` / `getnewblockhex` | OUI si `test_block_validity` (défaut true) |
| `signblock` wallet | signataire | OUI (`TestBlockValidity` avant signe) |
| `combineblocksigs` | assembleur | **NON** (CheckProof seulement) |
| Réception P2P | nœuds | `ConnectBlock` → VerifyAmounts si `fScriptChecks` |

Donc : **PRODUCER CAN BUILD/ASSEMBLE INVALID BLOCK = CONFIRMED** (chemin `combineblocksigs` / injection hors mempool).  
**≠** « le producteur a accepté via ConnectBlock ».

---

## 5. VERSION / CONFIGURATION ANALYSIS

| NODE / ROLE | VERSION | COMMIT | BUILD | CONFIG | SOURCE | DATE | CONFIDENCE |
|---|---|---|---|---|---|---|---|
| Fonctionnaire prod 2026-09-06 | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | — | — | — |
| Producteur 4050336/7 | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | — | — | — |
| Blockstream Liquid API backend | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | API seule | — | — |
| Tree Elements (lab) | clone local | dont tag `elements-23.3.3` / fix cache `c26d719` | lab | defaults | git | tags connus | HIGH pour le tree, **pas** pour la prod |

**INTERDICTION respectée :** aucune déduction « dernière release ⇒ version prod ».

---

## 6. VALIDATION CALL GRAPH

### STOCK (attendu, cache vide, tip)

```
4050336
 ↓ CheckBlock          → PASS (pas VerifyAmounts)
 ↓ TestBlockValidity / ConnectBlock
 ↓ fScriptChecks=true  (assumevalid Liquid défaut = null)
 ↓ CheckTxInputs(f24a)
 ↓ VerifyAmounts
 ↓ CRangeCheck(P1,C1)
 ↓ secp256k1_rangeproof_verify → FALSE
 ↓ REJECT
```

### RÉEL OBSERVÉ

```
4050336
 ↓ CheckProof / 11-of-15   → PASS   [FACT]
 ↓ ?
 ↓ 4050337.prev = e1d9     [FACT]
 ↓ … → 4050343             [FACT]
```

Matrice lab (pas de nœud prod instrumenté) :

| ENVIRONMENT | VERSION | CACHE | ASSUMEVALID | fScriptChecks | CheckBlock | TestBlockValidity | ConnectBlock | ActivateBestChain | RESULT |
|---|---|---|---|---|---|---|---|---|---|
| Lab stock model | Elements tree | empty | null | true | PASS | FAIL (model) | FAIL (model) | n/a | REJECT f24a |
| Lab + primed cache | same | hit P1\|\|C1 | null | true | PASS | PASS (lab) | PASS (lab) | n/a | ACCEPT lab ; hist prime **UNSUPPORTED** |
| Lab assumevalid skip | same | any | set + ancestor gate | false | PASS | skip amounts | may ACCEPT | n/a | code-only ; **not proven in prod** |
| Production tip builder | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN |

---

## 7. FIRST REAL DIVERGENCE

**Premier écart observable (pas une théorie) :**

| | |
|---|---|
| Attendu stock | `ConnectBlock` refuse f24a (RP FAIL) |
| Observé | ancêtre signé + enfants jusqu’à ≥4050343 + UTXO f24a:0 dépensé en 4050337 |

**Localisation du « ? » :** entre  
`CheckProof(4050336)=OK`  
et  
« état UTXO / tip permettant 4050337 ».  

**Pas identifié :** lequel de `VerifyAmounts` skip, cache hit, chemin hors `TestBlockValidity`, validateur custom, ou autre.

---

## 8. ALTERNATIVE EXPLANATIONS

| # | Mécanisme | Peut skip VerifyAmounts ? | CRangeCheck ? | RP verify ? | Défaut Liquid ? | Prouvé actif ? | Peut expliquer 4050336 ? |
|---|---|---|---|---|---|---|---|
| assumevalid → `fScriptChecks=false` | OUI | OUI | OUI | **NON** (`defaultAssumeValid` vide) | NON | OUI en code, **pas** en incident |
| rangeproof cache hit | NON (toujours appelé) | skip crypto si hit | skip | défaut cache vide | hist prime **UNSUPPORTED** | lab only |
| `combineblocksigs` sans TestBlockValidity | n/a (pas Connect) | — | — | n/a | UNKNOWN usage | explique **signature**, pas tip |
| OP_RETURN skip RP | NON | NON | NON | — | REFUTED | NON |
| `block.fChecked` / reindex | NON pour amounts | NON | NON | — | — | NON |
| BLOCK_VALID_SCRIPTS sans Connect | NON | — | — | — | — | REFUTED (flag après Connect OK) |
| assumeutxo / snapshot | UNKNOWN prod | UNKNOWN | UNKNOWN | — | UNKNOWN | UNKNOWN |
| Custom / HSM non-Elements | UNKNOWN | UNKNOWN | UNKNOWN | — | UNKNOWN | UNKNOWN |

---

## 9. FACT / INFERENCE / HYPOTHESIS

| Item | Tag |
|---|---|
| f24a dans 4050336 ; P1 RP FAIL indépendant | FACT |
| Seule RP FAIL du bloc = f24a:1 | FACT (`block_4050336_tx_rp_scan.json`) |
| 11-of-15 ; positions listées §3 | FACT |
| `4050337.prev == e1d9` … chaîne à 4050343 | FACT |
| Stock empty-cache → REJECT | FACT (code + repro) |
| Au moins un tip a traité e1d9 comme parent utilisable | INFERENCE (chaîne + spend f24a:0) |
| Premier accepteur nommé | UNKNOWN |
| Cache / assumevalid / fScriptChecks=false en prod | HYPOTHESIS sans preuve |

---

## 10. CONFIRMED

- Chaîne parent 4050335→4050343 cryptographiquement cohérente (API hashes).  
- Matching ECDSA signblock → positions pubs (tous blocs de la série).  
- Intersection signataires 36∩37 = 10 pubs.  
- Stock path : `ConnectBlock` → `VerifyAmounts` si `fScriptChecks`.  
- `fScriptChecks` mis à false **uniquement** via porte assumevalid (`validation.cpp` ~2851–2877).  
- Liquid défaut assumevalid = null → ne peut pas expliquer tip **par défaut**.  
- `combineblocksigs` ne vérifie pas les amounts.  
- Producer workflow peut assembler un bloc invalid sans ConnectBlock réussi.  
- f24a:1 = seule RP FAIL du bloc.

---

## 11. PROBABLE

- Sous prémisses stock + cache vide + mempool + `test_block_validity=true`, f24a n’entre pas via `getnewblockhex`/`signblock` wallet — donc construction/signature a **probablement** utilisé un chemin hors ce pipeline (ex. injection + `combineblocksigs`).  
  (*Probable logique code, pas log producteur.*)

---

## 12. PLAUSIBLE

- Tip-building node avec config non-défaut (`-assumevalid`…) ou état cache différent.  
- Indexer Blockstream distinct d’un full ConnectBlock (mécanisme UNKNOWN).

---

## 13. UNKNOWN

- **FIRST ACCEPTOR**  
- Version / config / cache de cet environnement  
- Qui a construit 4050336 / produit 4050337  
- Preuve d’un rejet concurrent (split)  
- Preuve historique de cache hit P1/C1  
- Preuve runtime assumevalid / `fScriptChecks=false` le 2026-09-06  
- BLOCKSTREAM ACCEPTANCE MECHANISM  

---

## 14. REFUTED

- « assumevalid défaut Liquid explique l’acceptation au tip »  
- « OP_RETURN skip la rangeproof confidentielle »  
- « Affichage Blockstream seul = preuve ConnectBlock »  
- « Signature fédération = chaque signataire a ConnectBlock’d »  
- Cache poisoning comme **mécanisme historique démontré** pour cet incident (lab A→B ≠ preuve P1/C1)  
- Déduire la version prod de la « dernière release »

---

## 15. CAUSAL GRAPH

```
FACT: f24a existe, contient P1
  ↓
FACT: P1 échoue rangeproof_verify (L-BTC)
  ↓
FACT: f24a inclus dans 4050336
  ↓
FACT: 4050336 signé 11-of-15 (pubs positions connues)
  ↓
UNKNOWN: premier ConnectBlock(4050336) réussi  ← PREMIER LIEN NON DÉMONTRÉ
  ↓
FACT: 4050337.prev = hash(4050336)
  ↓
FACT: … → 4050343
```

---

## 16. SCORES (preuves de cette mission uniquement)

| ID | Hypothèse | Score 0–100 | Note |
|---|---|---:|---|
| H1 | cache poisoning | **12** | bug lab CONFIRMED ; hist prime absente |
| H2 | version mismatch | **22** | aucune version prod pinée |
| H3 | config / fScriptChecks | **35** | seule voie code connue hors cache ; **aucune** preuve config prod |
| H4 | assumevalid | **18** | défaut Liquid la contredit ; runtime UNKNOWN |
| H5 | producer workflow (sans TestBlockValidity) | **55** | code permet ; usage incident UNKNOWN ; n’explique pas seul le tip |
| H6 | autre bug consensus/validation | **28** | pas de second bug localisé ici |
| H7 | consensus split | **8** | aucune preuve NodeA REJECT / NodeB ACCEPT |
| H8 | combinaison | **40** | plausible a priori ; non démontré |

---

## VERDICT FINAL A–N

| # | Question | Réponse |
|---|---|---|
| **A** | Qui a construit 4050336 ? | **UNKNOWN** |
| **B** | Qui l’a signé ? | 11 pubs positions **0,1,3,4,5,6,7,8,11,13,14** (CONFIRMED ECDSA) ; pas d’identité humaine |
| **C** | Qui a produit 4050337 ? | **UNKNOWN** (signataires : 0,1,3,5,6,7,8,10,11,13,14) |
| **D** | Premier environnement ayant accepté 4050336 ? | **FIRST ACCEPTOR = UNKNOWN** |
| **E** | Quelle version ? | **UNKNOWN** |
| **F** | Quelle configuration ? | **UNKNOWN** |
| **G** | Quel chemin exact de code a accepté ? | **UNKNOWN** (`?` après CheckProof) |
| **H** | Une autre version rejette-t-elle ? | Lab/stock model : **OUI** (REJECT) ; autre version prod : UNKNOWN |
| **I** | Une autre config rejette-t-elle ? | Défaut tip + cache vide : **OUI** ; configs prod : UNKNOWN |
| **J** | Consensus split réel ? | **CONSENSUS SPLIT = UNKNOWN** |
| **K** | Cache nécessaire ? | **Non prouvé** |
| **L** | Cache suffisant ? | Suffisant **en lab** avec priming ; **pas montré** historiquement |
| **M** | f24a causal ? | Seule RP invalide du bloc ; présent sur l’ancêtre du tip ; **causalité consensus non prouvée** au-delà de « contenu du bloc signé » |
| **N** | Premier lien causal non démontré ? | **`ConnectBlock(4050336)` accepté par un runtime identifiable** |

### f24a nécessaire au contenu signé ?

- Seule RP FAIL du bloc : **OUI** (scan).  
- Retirer/remplacer f24a change le merkle root → les **signatures dynafed (sur block hash) deviennent invalides** : les 11 sigs authentifient **exactement** ce contenu (header+txs).  
- Anomalie = **contenu autorisé** par le quorum, pas un artefact hors bloc.

### Cache (mission 10)

| | |
|---|---|
| A preuve cache hit environnement réel | **NON** → CACHE USED IN INCIDENT = **UNKNOWN** |
| B hit sans priming historique P1/C1 | non démontré |
| C producteur cache différent | UNKNOWN |
| D logs cache hit | aucun trouvé |

### Trace d’acceptation (mission 12)

| TIMESTAMP | ACTOR | EVENT | SOURCE | CONFIDENCE |
|---|---|---|---|---|
| 1788702790 | explorer index | 4050336 | Blockstream API | FACT (champ) |
| 1788702850 | explorer index | 4050337 | Blockstream API | FACT |
| — | nœud | ConnectBlock / reject | — | **UNKNOWN** (aucun log) |

Messages médias (SideSwap / Blockstream ~2026-09-06) décrivent peg-out / bug Elements — **ne pinent pas** le premier `ConnectBlock` de 4050336.

### Blockstream

**BLOCKSTREAM ACCEPTANCE MECHANISM = UNKNOWN.**  
Preuve indépendante de poursuite de branche : `4050337.prev` … `4050343.prev` (pas l’UI seule).
