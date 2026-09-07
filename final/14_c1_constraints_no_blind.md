# 14 — Contraintes sur v1 sans blind (indépendant du cache)

## Verdict

Sur les commitments et valeurs **connus** de `f24a`, **aucune propriété anormale de C1 n’est démontrable** (signe, borne inférieure, hors-domaine rangeproof).  
L’équation de conservation **n’est pas intrinsèquement inflationnaire** : elle est compatible avec un `v1` parfaitement normal.  
P1 échoue, donc l’in-range de `v1` est **non prouvé**, pas **réfuté**.

**Conclusion : D — Aucune inflation démontrée.**

Logs : `final/logs/c1_constraint_probe.log`, `final/logs/pedersen_tally_f24a.log`

---

## 1. Équation de conservation (FACT)

Comme points de courbe (Pedersen) :

```
C_in0 = C_out0 + C1 + C_out2 + C_fee58
```

Exécuté : `pedersen_verify_tally` → **1** ; sans C1 → **0**.

Aucun issuance. Une seule entrée.

Sous générateurs d’asset :

```
C_in0  = v_in · Gen_in  + r_in · H
C_out0 = v0   · G_L    + r0   · H     (asset L-BTC explicite)
C1     = v1   · G_L    + r1   · H     (asset L-BTC explicite)
C_out2 = v2   · Gen_2  + r2   · H     (asset blinded ; surjection ↔ Gen_in)
C_fee  = 58   · G_L    + 0    · H
```

avec `G_L = generator_generate(L-BTC)`  
`= 0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d`

---

## 2. Séparation

| Classe | Éléments |
|---|---|
| Valeurs explicites | fee = **58** sats L-BTC uniquement |
| Commitments confidentiels (valeur) | `C_in0`, `C_out0`, **C1**, `C_out2` |
| Assets explicites | out0, out1, fee → L-BTC |
| Assets confidentiels | input, out2 (surjection **TRUE**, 67 B) |
| Fee | out3, 58, blind 0 |
| Générateurs | `G_L` explicite ; `Gen_in` / `Gen_2` éphémères liés par surjection |

---

## 3–4. Quelles valeurs sont connues ?

| Quantité | Statut |
|---|---|
| `v_in`, `r_in` | **inconnus** (prevout `0fbde521…:2` entièrement confidential) |
| asset ID sous-jacent de l’input | **non déterminable** (pas d’explicit, pas de surjection depuis un L-BTC explicite) |
| `v0`, `r0` | **inconnus** (out0 confidential ; RP out0 **valide** ⇒ seulement `v0 ∈ [0, 2^64)` prouvé) |
| `v2`, `r2`, blinds d’asset | **inconnus** (RP out2 valide ⇒ `v2` in-range) |
| fee | **58** connu |
| `v1`, `r1` | **inconnus** ; on conserve `C1 = v1 G_L + r1 H` sans inventer `v1` |

Aucune ouverture récupérable depuis les prevouts publics.

---

## 5–7. Contraintes extractibles sur `v1` ?

**Méthode résiduelle (points) :**  
`C1 = C_in0 − C_out0 − C_out2 − C_fee`  
est vrai comme identité de groupe (tally). Cela **fixe le point C1**, pas le scalaire `v1`.

**Représentation unique :**  
Pour `(G_L, H)` fixés, il existe un unique couple `(v1, r1) ∈ F_n²` tel que `C1 = v1 G_L + r1 H`.  
Le calculer = log discret → **non déterminable**.

**Piège à éviter (argument faux « indépendance de générateurs ») :**  
Dans un groupe cyclique, on ne peut **pas** lire  
`v0 + v1 + 58 ≡ 0 (mod n)`  
à partir du tally multi-asset sans ouvertures / DLOG. Cet argument **ne démontre pas** `v1` négatif.

**Sonde exécutée :** `C1 ≠ commit(v, r=0, G_L)` pour tout `v ∈ [0, 100000]`  
⇒ ce n’est pas un montant « explicite déguisé » à blind nul sur cette plage.  
Cela **n’implique pas** hors-domaine.

| Propriété | Démontrable ? |
|---|---|
| `v1` doit être positif | **non** |
| `v1` > une borne | **non** |
| `v1` doit être négatif | **non** — interdit de l’affirmer |
| `v1` hors domaine rangeproof | **non** |
| Aucune de ces propriétés | **oui** |

---

## 8–9. Tally seul vs tally + rangeproof

| Mode | Ce qui est garanti |
|---|---|
| **A. Tally seul** | Égalité de points `Σ C_in = Σ C_out`. Aucune garantie que chaque `v` soit dans `[0, 2^64)`. |
| **B. Tally + rangeproofs** | + chaque sortie confidentielle ouvre à un `v` in-range sous son générateur (et politique Elements). |

**Propriété supplémentaire de la rangeproof :**  
preuve ZK que le scalaire caché `v` de `C = v·Gen + r·H` est dans l’intervalle annoncé (ici info P1 : `min=0`, `max≈2^52`, `exp=0`, `mantissa=52`) **sans révéler** `(v,r)`.

Sans RP valide sur C1 : le tally peut tenir avec un `v1` in-range **ou** hors-range ; les deux sont indiscernables depuis les points seuls.

---

## 10. Pourquoi P1 échoue — trichotomie

Exécuté : `rangeproof_info(P1) = OK` (structure / bornes annoncées) ;  
`rangeproof_verify(C1, P1, G_L, script=OP_RETURN) = FALSE` ;  
échecs aussi sous d’autres scripts / generators testés (batch antérieur : 0 hit).

| Hypothèse | Statut |
|---|---|
| C1 représente réellement une valeur hors domaine | **possible, non démontré** |
| P1 est une preuve incorrecte / non liée à C1 | **possible, non démontré** |
| P1 a été générée sous un autre contexte (Gen / extra_commit) | **possible, non démontré** (contexte exact **non trouvé**) |

**Non déterminable** laquelle est vraie. L’échec de verify **ne sélectionne pas** « hors domaine ».

---

## 11–13. Interdits et test « v1 normal »

- On **n’affirme pas** « C1 est négatif ».
- On **n’affirme pas** « X BTC créés ».

**L’équation peut-elle être satisfaite par un C1 à valeur normale ?**  
**Oui, structurellement.** Un spend L-BTC confidentiel → change + sortie OP_RETURN dust + fee produit la même forme de tally avec `v1` petit et in-range.  
Pour **ce** C1 publié, savoir si son unique `v1` est normal reste **non déterminable** ; rien dans le tally ne l’exclut.

---

## Indépendance du cache

Cette note n’utilise **aucune** hypothèse sur le cache de rangeproofs.  
Question tranchée : *la conservation de valeur de f24a, vue depuis les commitments publics, force-t-elle une anomalie sur C1 ?*  
**Non.**

---

## Conclusion obligatoire

### D — Aucune inflation démontrée

**Pourquoi pas A :** aucun surplus chiffré, aucun `v1` ouvert.  
**Pourquoi pas B :** aucune contrainte mathématique extractible forçant `v1` hors domaine / négatif.  
**Pourquoi pas C comme verdict principal :** « inflation possible » est le résidu générique de tout commitment sans RP valide ; ici le tally **n’exerce aucune pression** vers l’inflation, et un `v1` normal reste compatible.  
**D** : au niveau conservation de valeur, f24a n’est **pas** démontrée anormale ; l’unique anomalie crypto locale reste **P1 invalide sous le contexte L-BTC+`6a`**, ce qui invalide la *preuve* d’in-range, sans démontrer une *valeur* inflationnaire.
