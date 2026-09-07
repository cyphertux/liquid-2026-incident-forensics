# 09 — Timeline (UTC)

Block times from explorers are converted from unix timestamps in API JSON.

| Time (UTC) | Event | Class |
|---|---|---|
| 2026-08-03 | Fix authored (`c26d719` AuthorDate) | CODE |
| 2026-08-14 | secp256k1-zkp PR #369 merged (surjection nonce binding) | CODE |
| 2026-08-14 | secp256k1-zkp PR #370 merged (rangeproof nonce reuse warning) | CODE |
| 2026-08-19 | Elements PR #1582 merged (secp update) | CODE |
| 2026-04-13 | Release **elements-23.3.3** (still vulnerable cache key) | CODE |
| 2026-09-01 | `c26d719` committed + merged to master via #1592 | CODE |
| 2026-09-02 | Cherry-pick `6253d7e` onto 23.x lineage | CODE |
| 2026-09-03 | PR #1595 merged to elements-23.x | CODE |
| 2026-09-03 | Cherry-pick `212c43f` for 23.3.x | CODE |
| 2026-09-06 ~**17:21Z** | PR #1599 merged to elements-23.3.x | CODE |
| ~2026-09-06 (block_time 1788702610 → compute) | height 4050333 prev activity | ON-CHAIN |
| height **4050335** | txs `27114710…`, `71c93d43…` embed C1 in OP_RETURN | ON-CHAIN |
| height **4050336** / hash `e1d9…a0d5` | **f24a** confirmed (block_time 1788702790) | ON-CHAIN |
| height **4050344** | **46f117** peg-out 2.65138358 | ON-CHAIN |
| height **4050349** | **ce4cae** peg-out 3996.01834922 | ON-CHAIN |
| BTC height **965783** | **8db751…** pays both peg-out amounts | ON-CHAIN |
| 2026-09-06 public reporting | Liquid bridge paused / incident disclosures | SOURCE (news) |

Exact unix→UTC for Liquid block_time 1788702790:
`python: datetime.utcfromtimestamp(1788702790)` → **2026-09-06 14:13:10 UTC** (FACT via calculation).

## Note

PR #1599 (23.3.x backport of cache fix) merged **the same calendar day** as the incident. Whether production Liquid functionary nodes had already upgraded is **NOT VERIFIED**.
