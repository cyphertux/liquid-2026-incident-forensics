# Mission 39.1 — Génération exacte des artefacts (Mission 40)

**Verdict : NOT_READY**

**D déclaré : NON**

## Réponse à la question unique

> Peut-on maintenant présenter au véritable elementsd post-fix un primer et un alias contenant les octets exacts de Mission 28, dans un bloc **regtest** réellement sérialisé, sans modifier les règles de consensus ?

**NON** (au sens ConnectBlock / validation regtest).

Ce qui est disponible :

| Livrable | Statut | Niveau |
|---|---|---|
| `mission40/primer.hex` / `alias.hex` (Liquid CTransaction exact) | oui | FACT |
| Champs P0,C0,G,S0 / P1,C1,G,S1 bit-identiques Mission 28 | oui | FACT |
| `primer_block.hex` / `alias_block.hex` = blocs Liquid 4050335/4050336 (merkle/prev documentés dans `*_block.json`) | oui | FACT |
| Blocs **regtest** ConnectBlock-ready avec ces txs exactes | non | — |
| Reconstruction consensus-compatible | non produite | — |
| elementsd post-fix `ProcessNewBlock` | BLOCKED_BY_POSTFIX_BUILD | — |

## Classification épistémique

### FACT
- Offsets et égalités d’octets (voir `final/data/39_1_fixture_mapping.json`).
- `decoderawtransaction` liquidv1 OK ; regtest FAIL pour les mêmes hex.
- `preimage_primer == preimage_alias` (4301 B, sha256 `82b0b8ccf8c743171f2bc8d80bb9982a81cfca583e8dfbe65563cf095e99c01a`).
- Blocs bruts téléchargés contiennent les txs (offsets 10680 / 1802).

### EXPERIMENT
- `build/m391_crypto` : primer native_verify=**TRUE**, alias=**FALSE**.
- `build/28_cache_replay` (M28) : primer SET → alias HIT ACCEPT (fidélité checker, pas ConnectBlock).

### INFERENCE
- La collision de cache post-fix reste structurelle sur ces octets (M36) ; les artefacts EXACT permettent de la rejouer au locus checker.

### HYPOTHESIS
- Lien causal historique avec 4050336 / runtime des accepteurs : **non traité ici**.

## Dual version (règle §5)

- **EXACT FIXTURE** : `mission40/exact/` (+ copies top-level). Ne pas altérer.
- **CONSENSUS-COMPATIBLE** : `mission40/consensus_compatible/README.md` — **NOT_PRODUCED**.

## Primitives Elements utilisées

- Sérialisation réelle : txs/blocs Liquid historiques (mêmes structures `CTransaction` / block wire qu’elementsd).
- Decode : `elementsd` 23.3.3 `decoderawtransaction` (`-chain=liquidv1 -validatepegin=0`).
- Crypto : secp256k1-zkp bundlé (`build/secp256k1.o`) via `build/m391_crypto`.
- Pas de modification des règles de consensus ; pas de validateur patché pour forcer ACCEPT.

## f24a

Dossier `mission40/f24a/` — bytes non modifiés ; mapping Mission 28 documenté.

## Pourquoi NOT_READY malgré des hex « parfaits »

Les hex EXACT sont des artefacts **Liquid mainnet**, pas des blocs regtest acceptables par `ConnectBlock` sans prevouts/params.  
Mission 40 exige le chemin nodes regtest post-fix → **pas encore franchissable** ici.

## Critère Mission 40

Quand une reconstruction consensus-compatible (ou un autre chemin documenté) + binaire `3b3f01eac966…` seront disponibles : re-évaluer `READY_FOR_MISSION_40`.
