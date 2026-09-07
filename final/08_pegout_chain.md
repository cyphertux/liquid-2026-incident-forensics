# 08 — Peg-out chain (separate from proving the cache bug)

## FACT — Amounts

| Item | sats | BTC |
|---|---:|---:|
| `46f117…` peg-out | 265138358 | 2.65138358 |
| `ce4cae…` peg-out | 399601834922 | 3996.01834922 |
| **sum** | **399866973280** | **3998.66973280** |

## FACT — Linkage

1. `46f117c990580501a5156937a8c8affda551a38b3c0b99d29eb8469ee6beb3d2`  
   - height 4050344  
   - 13 inputs / 3 outputs  
   - out0: peg-out 2.65138358 L-BTC → BTC `bc1qkxwva32eh7mgezq5kladncd3n5wtcjmslh98my`  
   - out1: confidential (`096791c744…` / asset `0a4e9e7f…`)  
   - fee 225

2. `ce4caece413cd9d444ce7ed9f54e5b328b3da5e4af301aff59a3571f76e988f2`  
   - height 4050349  
   - **vin[4] spends `46f117…:1`** (exact outpoint match)  
   - out1: peg-out 3996.01834922 L-BTC → BTC `bc1qgslsydz56d0ed6827hdemfmk5w2f6ldyc6wt7p`  
   - fee 177

3. Bitcoin mainnet `8db751a650ae2f12006b7e8c69a75e4df360e8afd6b9e05ae0b9fa6458a7b140` (height 965783) pays:
   - 399601834922 → `bc1qgslsy…`
   - 265138358 → `bc1qkxwva…`
   - plus additional change/other outputs

## FACT vs INFERENCE

- On-chain peg-out amounts and BTC payout: **FACT**
- These peg-outs spend an L-BTC state that became available after block 4050336: **INFERENCE** from ordering (not a full UTXO provenance proof from f24a out1, which is OP_RETURN and unspendable)
- “Stolen 4000 BTC”: **not used** — prefer “BTC paid out via two peg-outs totaling 3998.66973280”

## Where is the bug relative to peg-out?

**INFERENCE:** if the cache bug allowed an invalid confidential state to be accepted earlier, later peg-outs can be cryptographically “normal” relative to that poisoned state.  
`ce4a` itself need not be malformed for the economic exit to occur.

Question “is the problem in ce4a or prior state?” → evidence favors **prior state / earlier acceptance** as the critical failure point; ce4a looks like a standard multi-input peg-out once inputs exist. **[INFERENCE]**
