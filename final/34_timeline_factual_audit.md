# Mission 34 — Audit factuel de la timeline « Liquid septembre 2026 »

**Date d’audit:** 2026-09-07  
**Règle:** FACT / INFERENCE / HYPOTHÈSE séparés. NON VÉRIFIÉ / NON DÉMONTRÉ si non prouvé.  
**Sources primaires consultées:** dépôt local `ElementsProject/elements`, GitHub API (`commits`, `pulls`), Blockstream Liquid Esplora, mempool.space Bitcoin, artefacts on-chain locaux (`final/data/*`).  
**Sources secondaires (citations seulement):** reprises de communiqué SideSwap via stacker.news / BTC Times / ChainCatcher (le post X `@side_swap` n’a pas été récupéré en raw ici → traité comme **déclaration SideSwap rapportée**, pas comme dump API SideSwap).

---

## 1. VERDICT GLOBAL

**Timeline partiellement correcte** — les événements on-chain ~13:53 / ~14:06 / ~14:28 UTC et le merge PR #1599 ~17:21 UTC sont juste ; plusieurs dates Git (ouverture PR, merge `master`, « 31 août », simplification du 4 septembre) sont **inexactes ou ambiguës**.

---

## 2. TABLEAU D’AUDIT

| Élément | Affirmation | Verdict | Preuve primaire | Heure/date exacte | Commentaire |
|---|---|---|---|---|---|
| 1. Création `c26d719` | « 3 août 2026, titre fix range proof cache… » | **PARTIEL** | `git show c26d719c29` ; API commit | **AuthorDate** `2026-08-03T10:53:50Z` (= 12:53:50 +0200) ; **CommitDate** `2026-09-01T09:39:02Z` | Le **titre** est exact. « Créé le 3 août » = date **auteur** seulement. L’objet entre dans l’historique publié au **CommitDate 1er sep**. |
| 2. Ouverture PR | « 31 août 2026 » | **INCORRECT** | [PR #1592](https://github.com/ElementsProject/elements/pull/1592) | **created_at `2026-09-01T10:04:36Z`** | Aucune PR « range proof cache » ouverte le 31 août trouvée. |
| 3. Merge dans `master` | « 2 septembre 2026 » | **INCORRECT** | PR #1592 `merged_at` ; merge commit `31e8f27f4b…` | **`2026-09-01T14:38:01Z`** | Mergé le **1er** septembre, pas le 2. |
| 4a. Backport `elements-23.x` | « 3 septembre » | **PARTIEL** | commit `6253d7e103` ; [PR #1595](https://github.com/ElementsProject/elements/pull/1595) | Cherry-pick **CommitDate** `2026-09-02T15:14:36Z` ; PR created `2026-09-02T15:46:40Z` ; **merged `2026-09-03T17:13:42Z`** | Le commit apparaît le **2** ; le merge de la PR de backports est le **3**. |
| 4b. Backport `elements-23.3.x` | « puis 23.3.x le 3 septembre » | **PARTIEL / À PRÉCISER** | commit `212c43f475` | **CommitDate** `2026-09-03T17:36:30Z` (−0700 = 10:36:30 local) | Cherry-pick présent sur la branche le **3** ; **pas** via le merge de #1599 (plus tard). |
| 5. 4 septembre « PR release / intégration 23.3.x / 23.3.4rc2 » | | **PARTIEL** | [PR #1599](https://github.com/ElementsProject/elements/pull/1599) | **Ouverture** `2026-09-04T19:36:51Z` ; **merge** `2026-09-06T17:21:03Z` | Le 4 = **ouverture** de PR « Cherry picks… in preparation for 23.3.4rc2 ». Ce n’est **pas** le merge, ni la publication d’un tag `23.3.4rc2` (tag **NON VÉRIFIÉ / absent** des tags listés). |
| 6. Bloc 4050336 / `f24a` ~13:53 | | **CORRECT** (affiner l’heure) | Blockstream Liquid API + `final/data/block_4050336.json` | **`2026-09-06 13:53:10 UTC`** | Hash `e1d9a2aa…a0d5` ; height **4050336** ; **7** txs ; `f24a…183f` **présente**. Prev = tip 4050335 @ **13:52:10 UTC**. |
| 7. `ce4cae…` peg-out ~14:06 / ~3996 BTC / SideSwap | | **PARTIEL** | Blockstream tx API ; SideSwap via sources secondaires | Bloc **`2026-09-06 14:06:10 UTC`** height **4050349** ; peg-out **exact 3996.01834922** BTC (= 399601834922 sats) → `bc1qgslsy…` | SideSwap déclare **14:05 UTC** / **4000 L-BTC** (rapportée) — proche mais **≠** timestamp de confirmation on-chain 14:06:10. Lien UTXO f24a→ce4 = **partiel confirmé** (vins 4 et 5 sur chemin f24a ; autres vins **UNKNOWN**). |
| 8. BTC `8db751…` ~14:28 / ~3996 | | **PARTIEL** | mempool.space | **`2026-09-06 14:28:56 UTC`** ; Bitcoin height **965783** | **vout0 = 3996.01834922** → `bc1qgslsy…` (**match exact** peg-out). La tx **dépense** le wallet fédé et paie aussi d’autres sorties (total outs ≈ 4019.44 BTC) — ce n’est pas « une seule sortie 3996 ». |
| 9. Merge correctif `23.3.x` ~17:21 | | **PARTIEL** | PR #1599 merge | **`2026-09-06T17:21:03Z`** ; merge commit `3b3f01eac9` | Merge **après** l’incident. Mais le patch `212c43f` était **déjà** sur `elements-23.3.x` depuis le **3**. Relation `23.3.4rc2` = intention déclarée dans le body PR ; **tag/release 23.3.4rc2 NON VÉRIFIÉ**. |
| 10. `elements-23.3.3` | date / contient fix ? | **CORRECT sur l’essentiel** | GitHub release + `git merge-base` | Tag annoté **2026-04-10** ; release **published `2026-04-13T16:21:46Z`** | **`c26d719` / `6253d7e` / `212c43f` ABSENTS** de `elements-23.3.3` et de `elements-23.3.4rc1`. Dernière release publique taguée listée avant incident = **pré-fix**. |

---

## 3. TIMELINE CORRIGÉE (vérifiée uniquement)

Toutes les heures en **UTC** sauf mention contraire.

### A. Correctif Elements (GitHub)

| Quand (UTC) | Quoi | ID |
|---|---|---|
| **2026-08-03 10:53:50** | **AuthorDate** du changement « fix: range proof cache bind to asset and scriptpubkey » (Byron Hambly / `delta1`) | hash final `c26d719c29…` |
| **2026-09-01 09:39:02** | **CommitDate** de `c26d719` (entrée dans l’historique de la branche de PR) | `c26d719c29` |
| **2026-09-01 10:04:36** | Ouverture [PR #1592](https://github.com/ElementsProject/elements/pull/1592) « blind/blindpsbt fixes » — body: *« Fixes a number of small issues picked up during LLM scans »* ; base `master` | inclut `c26d719` parmi d’autres commits |
| **2026-09-01 13:46:32** | Review comment : `tomt1664` **ACK** `c26d719…` « tested locally » | pas d’autre review API |
| **2026-09-01 14:38:01** | **Merge** PR #1592 → `master` | merge `31e8f27f4b…` |
| **2026-09-02 15:14:36** | Cherry-pick équivalent sur ligne 23.x : `6253d7e103` (Committer Tom Trevethan) | même AuthorDate 3 août |
| **2026-09-02 15:46:40** | Ouverture [PR #1595](https://github.com/ElementsProject/elements/pull/1595) « elements-23.x: backports from master » | |
| **2026-09-03 16:09:18** | Review `delta1` APPROVED sur #1595 | |
| **2026-09-03 17:13:42** | **Merge** PR #1595 → `elements-23.x` | |
| **2026-09-03 17:36:30** | Cherry-pick `212c43f475` sur `elements-23.3.x` (« cherry picked from 6253d7e… », Committer Pablo Greco) | **contenu déjà sur 23.3.x** |
| **2026-09-04 19:36:51** | Ouverture [PR #1599](https://github.com/ElementsProject/elements/pull/1599) « Cherry picks from 23.x into 23.3.x » — *« in preparation for 23.3.4rc2 »* | head contient déjà `212c43f` |
| **2026-09-05 17:55:25** | ACK `tomt1664` sur #1599 | |
| **2026-09-06 17:20:37** | ACK `delta1` sur #1599 | |
| **2026-09-06 17:21:03** | **Merge** PR #1599 → `elements-23.3.x` (après l’incident on-chain) | merge `3b3f01eac9` |

### B. Release publique

| Quand (UTC) | Quoi |
|---|---|
| **2026-04-10** (tagger) / **published 2026-04-13 16:21:46** | Release/tag **`elements-23.3.3`** — **pré-fix** (pas d’ancêtre `c26d719`/`212c43f`) |
| **2026-07-01** (commit tip tag) | Tag **`elements-23.3.4rc1`** — **pré-fix** également |
| | Tag/release **`elements-23.3.4rc2`** : **NON VÉRIFIÉ** (mentionné seulement dans le body de #1599) |

### C. Incident on-chain

| Quand (UTC) | Quoi | Preuve |
|---|---|---|
| **2026-09-06 13:52:10** | Bloc Liquid **4050335** | hash `aad24e4f…d66b` |
| **2026-09-06 13:53:10** | Bloc Liquid **4050336** — **7** txs — contient `f24a4b17…183f` | hash `e1d9a2aa…a0d5` |
| **2026-09-06 14:06:10** | Bloc Liquid **4050349** — tx `ce4caece…88f2` peg-out **3996.01834922** L-BTC → BTC `bc1qgslsydz56d0ed6827hdemfmk5w2f6ldyc6wt7p` | hash `90a8c1ae…4730` |
| ~**14:05** (déclaratif) | SideSwap : client envoie **4000 L-BTC** au service peg-out | déclaration SideSwap **rapportée** (X/telegram via secondaires) |
| **2026-09-06 14:28:56** | Bitcoin bloc **965783** — tx `8db751a6…b140` : **vout0 3996.01834922** → même adresse hop ; autres outs (dont 2.65138358, etc.) | mempool.space |

**Relation f24a → peg-out (FACT partiel):** des descendants de `f24a:0` / `f24a:2` convergent vers `46f117` / `3289` puis alimentent **certains** inputs de `ce4` (vin4, vin5). **Tous** les inputs de `ce4` ne sont **pas** démontrés issus de `f24a`.

**Comportement des nodes face à 4050336 :** identité / version du premier acceptor = **UNKNOWN** (pas de log `ConnectBlock` public). Récits « explorer a rejeté » = **reporting secondaire**, pas preuve primary.

---

## 4. CE QUI EST CERTAIN (FACTS CONFIRMÉS)

1. Le message de commit / titre de `c26d719` est bien *fix: range proof cache bind to asset and scriptpubkey*.
2. Le patch passe `ComputeEntryRangeProof` de 2 champs (`proof‖commit`) à 4 (`proof‖commit‖asset‖script`) **sans framing** (diff `sigcache.cpp`).
3. Auteur : **Byron Hambly** (`delta1`, Blockstream email). Reviews publiques visibles : **ACK `tomt1664`** (et ACK `delta1`/`tomt1664` sur backports).
4. Le fix est bundlé dans PR #1592 avec d’autres correctifs blind/dynafed, présenté comme *small issues* issus de **LLM scans** — **pas** comme advisory « critical / exploit writeup ».
5. Équivalents : `c26d719` (master), `6253d7e` (23.x), `212c43f` (23.3.x).
6. **`elements-23.3.3` et `23.3.4rc1` ne contiennent pas le fix.**
7. Au moment de l’incident (~13:53 UTC), le **code source** post-fix était déjà sur branches publiques `master` / `23.x` / `23.3.x` (via `212c43f` depuis le 3).
8. Merge PR #1599 = **17:21:03 UTC le 6** — **après** 4050336.
9. Timestamps Liquid/Bitcoin ci-dessus (13:53:10 / 14:06:10 / 14:28:56) et montants peg-out/BTC **3996.01834922** sont on-chain.
10. Match satoshi peg-out Liquid → vout0 Bitcoin = **exact**.

---

## 5. CE QUI RESTE INCONNU / NON DÉMONTRÉ

| Question | Statut |
|---|---|
| Pourquoi AuthorDate = 3 août (travail local / amend / rebase) ? | **NON VÉRIFIÉ** (seul AuthorDate/CommitDate observables) |
| Scénario d’exploitation décrit dans PR/comments ? | **NON** — aucun writeup d’exploit dans #1592/#1595/#1599 (API) |
| Correctif traité comme *security-critical* avant le 6 ? | **NON DÉMONTRÉ** (langage public = « small issues » / LLM scans) |
| Version Elements déployée chez functionaries au moment de 4050336 | **UNKNOWN** |
| Le bug historiquement exploité est-il exactement le pré-fix incomplete keying, le post-fix aliasing (Mission 28), ou autre ? | **NON DÉMONTRÉ** |
| SideSwap a-t-il publié les txids `ce4` / `8db751` dans le communiqué ? | **NON VÉRIFIÉ** ici (communiqué parle d’heures/montants, pas forcément des hashes) |
| Tag/release `23.3.4rc2` existant au 6–7 sep | **NON VÉRIFIÉ** |
| liquid.network API | **404** lors de cet audit (divergence d’endpoint ; Blockstream Esplora OK) |
| Qui a accepté en premier le bloc 4050336 | **UNKNOWN** |

---

## 6. IMPLICATION POUR UNE PUBLICATION X (formulation prudente)

> Le 6 septembre 2026 à **13:53:10 UTC**, le bloc Liquid **4050336** a inclus la transaction `f24a…`. Plus tard le même jour, un peg-out on-chain de **3996.01834922** BTC (`ce4…` @ **14:06:10 UTC**) a été suivi d’une transaction Bitcoin (`8db751…` @ **14:28:56 UTC**) payant ce montant à `bc1qgslsy…`.  
> Indépendamment, le correctif public Elements `c26d719` / équivalents (liaison du cache rangeproof à asset+scriptPubKey) a une **AuthorDate** au **3 août 2026**, a été **mergé dans `master` le 1er septembre**, backporté sur **23.x / 23.3.x les 2–3 septembre**, alors que la release taguée **`elements-23.3.3` (avril 2026) ne le contient pas**.  
> **Cela établit une proximité temporelle entre code public et incident ; cela ne démontre pas** que les exploitants ont découvert le bug via GitHub, ni que les functionaries tournaient ce commit, ni que `c26d719` est le mécanisme exact de l’exploitation historique.

---

## 7. LIEN `c26d719` ↔ EXPLOITATION — trois niveaux

### FACTS CONFIRMÉS
- Dates Git / branches / absences dans `23.3.3` / timestamps blocs & txs (ci-dessus).
- Le correctif change **explicitement** le keying du cache rangeproof (asset + scriptPubKey).
- PR d’origine cadre le travail comme correctifs issus de **LLM scans**, pas comme disclosure d’incident.

### INFERENCES RAISONNABLES
- La chronologie est **compatible** avec une vulnérabilité de keying de cache rangeproof déjà connue/corrigée en amont dans le dépôt public **avant** l’incident.
- La dernière **release taguée** largement distribuée (`23.3.3`) étant pré-fix, il est **raisonnable** de penser que beaucoup de déploiements « stock » restaient pré-fix — **sans prouver** l’état des functionaries.
- Le merge #1599 post-incident **ne doit pas** être lu comme « le patch n’existait pas sur 23.3.x avant 13:53 » : `212c43f` était déjà là le 3.

### HYPOTHÈSES NON PROUVÉES
- Les attaquants ont trouvé le bug **via** le commit/PR GitHub.
- Les functionaries avaient déjà déployé `c26d719`/`212c43f` avant 4050336.
- Les functionaries savaient que ce commit était *the* bug exploitable.
- `c26d719` (ou Mission 28 post-fix aliasing) est **définitivement** le bug historiquement exploité.
- « Le correctif existait avant → donc l’attaque vient de ce correctif » — **invalidé comme raisonnement** ; au mieux une hypothèse à tester.

---

## 8. NOTES SUR PR #1592 (LLM scans, reviews, exploit)

| Question | Résultat |
|---|---|
| Pourquoi créé le 3 août ? | **NON VÉRIFIÉ** au-delà de l’AuthorDate |
| Problème décrit | Titre commit seulement ; body PR : lot de *small issues* LLM scans — **pas** de description narrative du cache |
| Qui | Auteur `delta1` / Byron Hambly ; ACK `tomt1664` |
| Commentaires | ACK test local ; *(« test each commit failing on master, unrelated »)* |
| Mention « LLM scans » | **Oui**, body PR #1592 |
| Scénario d’exploitation | **Absent** des commentaires/reviews API |
| Security-critical avant le 6 ? | **NON DÉMONTRÉ** |

---

## 9. SOURCES (priorité primaire)

**GitHub Elements**
- https://github.com/ElementsProject/elements/commit/c26d719c29a40da280a825b25657e9c3d8bc7d99
- https://github.com/ElementsProject/elements/commit/6253d7e103655ec015097de1505b5f3785ff6447
- https://github.com/ElementsProject/elements/commit/212c43f475fc202b5b9e6dbb1f1c616e1a06a6f7
- https://github.com/ElementsProject/elements/pull/1592
- https://github.com/ElementsProject/elements/pull/1595
- https://github.com/ElementsProject/elements/pull/1599
- https://github.com/ElementsProject/elements/releases/tag/elements-23.3.3

**Liquid / Bitcoin**
- https://blockstream.info/liquid/block/e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5
- https://blockstream.info/liquid/tx/f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f
- https://blockstream.info/liquid/tx/ce4caece413cd9d444ce7ed9f54e5b328b3da5e4af301aff59a3571f76e988f2
- https://mempool.space/tx/8db751a650ae2f12006b7e8c69a75e4df360e8afd6b9e05ae0b9fa6458a7b140

**Artefacts locaux recoupés**
- `final/data/block_4050336.json`, `pegout_flow.json`, `pegout_summary.json`, `mission32_git_history.json`, `22_full_onchain_flow.md`

**Secondaires (SideSwap rapporté)**
- Citations du communiqué SideSwap (14:05 / 14:28 / 3996 BTC) via stacker.news item 1563706 / BTC Times — à remplacer par capture primaire `@side_swap` si publication.

---

**FIN AUDIT — rien ci-dessus ne doit être lu comme preuve de causalité historique `c26d719` → 4050336.**
