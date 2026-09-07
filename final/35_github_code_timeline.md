# Timeline GitHub / code — rangeproof cache binding

**Périmètre:** dépôt public Elements + port Sequentia. Pas d’on-chain ici (voir `34_timeline_factual_audit.md` §C).  
**Règle:** heures en **UTC**. FACT seulement sauf mention.  
**Patch:** `ComputeEntryRangeProof` 2-arg (`proof‖value`) → 4-arg (`proof‖value‖asset‖scriptPubKey`). Framing advisory : **aucun** dans Elements.

---

## Chronologie

| Quand (UTC) | Événement | ID / lien |
|---|---|---|
| **2026-04-10** (tag) / **2026-04-13 16:21:46** (release) | Release **`elements-23.3.3`** — **pré-fix** | [release](https://github.com/ElementsProject/elements/releases/tag/elements-23.3.3) |
| **2026-07-01** (tip tag) | Tag **`elements-23.3.4rc1`** — **pré-fix** | tag local / GitHub |
| **2026-08-03 10:53:50** | **AuthorDate** du fix rangeproof cache (Byron Hambly / `delta1`) | hash final `c26d719c29…` |
| **2026-09-01 09:39:02** | **CommitDate** `c26d719` sur la branche de PR | [c26d719](https://github.com/ElementsProject/elements/commit/c26d719c29a40da280a825b25657e9c3d8bc7d99) |
| **2026-09-01 10:04:36** | Ouverture [PR #1592](https://github.com/ElementsProject/elements/pull/1592) « blind/blindpsbt fixes » — body : *LLM scans* / *small issues* ; base `master` | inclut `c26d719` + autres |
| **2026-09-01 13:46:32** | Review : `tomt1664` **ACK** `c26d719` (« tested locally ») | PR #1592 |
| **2026-09-01 14:38:01** | **Merge** #1592 → `master` | merge `31e8f27f4b…` |
| **2026-09-02 15:14:36** | Cherry-pick → `elements-23.x` | [6253d7e103](https://github.com/ElementsProject/elements/commit/6253d7e103655ec015097de1505b5f3785ff6447) (committer Tom Trevethan) |
| **2026-09-02 15:46:40** | Ouverture [PR #1595](https://github.com/ElementsProject/elements/pull/1595) « elements-23.x: backports from master » | |
| **2026-09-03 16:09:18** | Review `delta1` **APPROVED** sur #1595 | |
| **2026-09-03 17:13:42** | **Merge** #1595 → `elements-23.x` | |
| **2026-09-03 17:36:30** | Cherry-pick → `elements-23.3.x` | [212c43f475](https://github.com/ElementsProject/elements/commit/212c43f475fc202b5b9e6dbb1f1c616e1a06a6f7) (committer Pablo Greco ; « from 6253d7e ») |
| **2026-09-04 19:36:51** | Ouverture [PR #1599](https://github.com/ElementsProject/elements/pull/1599) « Cherry picks… preparation for **23.3.4rc2** » | head contient déjà `212c43f` |
| **2026-09-05 17:55:25** | ACK `tomt1664` sur #1599 | |
| **2026-09-06 13:53:10** | *(repère on-chain, hors Git)* bloc Liquid **4050336** / `f24a…` | — |
| **2026-09-06 17:20:37** | ACK `delta1` sur #1599 | |
| **2026-09-06 17:21:03** | **Merge** #1599 → `elements-23.3.x` (**après** incident) | merge `3b3f01eac9` |
| **2026-09-07 13:07:30** | AuthorDate commit Sequentia (même sémantique + **tests** `rangeproof_cache_binding_test`) | `3f0cf381404f` |
| **2026-09-07 13:12:12** | Ouverture [Sequentia PR #198](https://github.com/ConcatenaLabs/Sequentia/pull/198) | `albertodeluigi` |
| **2026-09-07 13:23:41** | **Merge** #198 → Sequentia `master` | merge `2b73be6390…` ; merged_by `albertodeluigi` |

Tag/release **`elements-23.3.4rc2`** : mentionné dans le body de #1599 seulement → **NON VÉRIFIÉ** (absent des tags listés au moment de l’audit).

---

## Équivalents du patch

| Commit | Branche | CommitDate (UTC) | Tests ajoutés |
|---|---|---|---|
| `c26d719c29` | `master` | 2026-09-01 | **non** (TODO blind_tests reste) |
| `6253d7e103` | `elements-23.x` | 2026-09-02 | **non** |
| `212c43f475` | `elements-23.3.x` | 2026-09-03 | **non** |
| `3f0cf38140` (Sequentia) | `master` | 2026-09-07 | **oui** (`blind_tests.cpp` +71) |

Pickaxe Elements : seuls ces **trois** commits introduisent `Write(asset_commitment)…Write(scriptPubKey)` dans `ElementsProject/elements`.

---

## État des branches au tip proche de l’incident (~2026-09-06 13:52Z)

| Branche | Tip (rev) | Contient le fix 4-arg ? |
|---|---|---|
| `master` | `c7e856fab1…` | **oui** via `c26d719` |
| `elements-23.x` | `323fc71625…` | **oui** via `6253d7e` |
| `elements-23.3.x` | `6c2c5626ba…` | **oui** via `212c43f` (déjà ancestor avant merge #1599) |
| `elements-29.x` | `d7789ac2e8…` | **non** (encore 2-arg au tip contrôlé) |

**Correction critique :** merge #1599 post-incident ≠ « patch absent de 23.3.x avant 13:53 ». Le contenu `212c43f` était sur la branche depuis le **3**.

---

## Releases taguées vs fix

| Tag | Contient le fix ? |
|---|---|
| `elements-23.3.3` | **non** |
| `elements-23.3.4rc1` | **non** |
| `elements-23.3.4rc2` | **NON VÉRIFIÉ** (pas de tag public trouvé) |

---

## Différences Sequentia #198 vs Elements

| | Elements `c26d719` | Sequentia #198 |
|---|---|---|
| Keying cache | identique (4-arg) | identique |
| Framing PR | « LLM scans » / small issues | titre *consensus* + attribution Liquid ~4000 BTC dans le body |
| Tests | aucun dans le commit | `rangeproof_cache_binding_test` (asset / script / issuance→spendable) |
| Rôle épistémique | primary (repo amont) | secondary (fork post-incident) |

---

## Ce que cette timeline **ne** prouve **pas**

- Que les functionaries tournaient un tip pré- ou post-fix le 6 sep  
- Que les attaquants ont découvert le bug via GitHub  
- Que `c26d719` (pré-fix incomplete keying) est **le** mécanisme historique de `f24a` / 4050336  
- Qu’un merge post-incident = patch inventé après coup (faux pour 23.3.x)

JSON machine-readable : `final/data/github_code_timeline.json`  
Audit factuel complet (Git + chain) : `final/34_timeline_factual_audit.md`
