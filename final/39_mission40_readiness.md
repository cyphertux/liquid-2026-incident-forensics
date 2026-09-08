# Mission 39 — Préparation intégrale de l’expérience D (Mission 40)

**Verdict Mission 39 : NOT READY — missing valid regtest `primer.hex` + `alias_block.hex` producers embedding exact Mission 28 bytes**

**D déclaré : NON** (interdit ici)

Le dispositif build / fixtures / nodes / instrumentation / checklist D est en place.  
Il manque encore les **artefacts transaction/bloc regtest** qui portent exactement `(P0,C0,G,S0)` / `(P1,C1,G,S1)` via le chemin normal — cela nécessite le binaire post-fix (ou un builder confidential hors-ligne non livré ici).

---

## DÉJÀ DÉMONTRÉ (Missions 36–38)

| Item | Statut |
|---|---|
| Collision structurelle post-`c26d719` | oui |
| Bypass `CachingRangeProofChecker` | oui |
| Locus `CheckTxInputs → VerifyAmounts → CRangeCheck` | oui (C) |
| `rangeProofCache` process-global | oui (code + init log) |
| Framing casse le bypass | oui (lab) |

---

## Commit immuable recommandé (Mission 40)

```text
RECOMMENDED_COMMIT = 3b3f01eac966afb266cd205b3a2cb87c8c1e4534
```

- Tip `elements-23.3.x` = merge **#1599**
- Contient `212c43f475` (fix rangeproof cache)
- Formule vérifiée :

```text
SHA256(salt || proof || value_commitment || asset_commitment || scriptPubKey)
```

**sans framing** (snippet Write 4 champs).

Minimal fonctionnel : `212c43f475fc202b5b9e6dbb1f1c616e1a06a6f7`  
Ancêtre master : `c26d719c29…`  
**Refuser** tout binaire `23.3.3` (pré-fix).

Pin : `mission40/COMMIT.txt`

---

## BLOQUÉ UNIQUEMENT PAR L’ENVIRONNEMENT / ARTEFACTS TX

| Blocage | Détail |
|---|---|
| Compilation post-fix | deps/cmake/autoconf absents ici — **script prêt** |
| `primer.hex` / `alias_block.hex` | fragments Liquid ≠ txs regtest valides — **TODO Mission 40** (`mission40_make_alias_block.sh` stub) |
| `ProcessNewBlock` mesuré | dépend des artefacts ci-dessus |

Voir `final/data/40_4050336_mapping.json` (ce qui serait perdu en « transformant » f24a pour regtest).

---

## PRÊT POUR MISSION 40 (checklist dispositif)

| Composant | Path | Vérifié |
|---|---|---|
| Build script | `scripts/mission40_build.sh` | oui (logique ; compile non exécutable ici) |
| Fixture verifier | `scripts/mission40_verify_fixtures.py` | **PASS** |
| Instrumentation | `scripts/mission40_instrument.sh` | oui |
| Framed control | `scripts/mission40_apply_framed.sh` | oui |
| Run orchestrator | `scripts/mission40_run.sh` | oui (exit 2 sans artefacts) |
| Verify D | `scripts/mission40_verify_D.sh` | oui (n’imprime jamais « D PROVEN ») |
| Nodes A/B conf | `mission40/nodeA|B/` | identiques (ports séparés) |
| Fixtures | `mission40/fixtures/` | copiées + check OK |
| Protocol | `mission40/PROTOCOL.md` | oui |
| Mapping 4050336 | `final/data/40_4050336_mapping.json` | oui |
| Manifest | `final/data/39_mission40_manifest.json` | oui |

### Fixture check (exécuté Mission 39)

```text
preimage_A == preimage_B (4301 bytes)
unframed keys equal
framed keys differ
P1 = P0||C0||G||6a||43
S0 = 6a||43||C1||G||6a
```

---

## Séquence Mission 40 (une fois machine de build dispo)

```bash
./scripts/mission40_verify_fixtures.py
./scripts/mission40_build.sh
./scripts/mission40_instrument.sh && ./scripts/mission40_build.sh   # rebuild instrumenté
# Implémenter producers → mission40/results/{primer.hex,alias_block.hex}
./scripts/mission40_run.sh
./scripts/mission40_verify_D.sh mission40/results
# Contrôle : framed build + même scénario
# Restart test: voir PROTOCOL.md
```

Si checklist verte → script affiche **`POTENTIAL D — REVIEW REQUIRED`** (revue humaine obligatoire).

---

## Ce que Mission 39 refuse

- Déclarer **D**
- Modifier le consensus pour forcer ACCEPT
- Prétendre que `submitblock` de `tx_f24a.hex` mainnet marche en regtest

---

## Une phrase

**Pack Mission 40 prêt côté build/fixtures/nodes/logs/critères ; pas READY tant que les hex primer/bloc regtest exacts ne sont pas produits — D reste pour Mission 40 après compilation post-fix.**
