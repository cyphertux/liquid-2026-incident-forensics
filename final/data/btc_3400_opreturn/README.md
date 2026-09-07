# BTC address OP_RETURN + 3400 BTC return

- Address: `bc1ql4mfu6aundtkksxklfajs2h3t9nzcd6gyqjlte`
- https://mempool.space/address/bc1ql4mfu6aundtkksxklfajs2h3t9nzcd6gyqjlte
- OP_RETURN outputs in address txs: **153** (71 unique payloads)

## 3400 BTC return

- **txid:** `a6d697a25266ce3c78774fd1d75f896b7af522ada209b0f6228ea497bc49a46d`
- https://mempool.space/tx/a6d697a25266ce3c78774fd1d75f896b7af522ada209b0f6228ea497bc49a46d
- block **965950**, 2026-09-07T16:09:25Z
- **vout1 = 3400.00000000 BTC** → `bc1qdlld6antmv4xug242ed83q7k4rqw50cwfns38szx4qu2f4jwaxxsuhwxxr`
- vout0 = 598.49955894 BTC change → whitehat addr
- fee 1768 sats; **no OP_RETURN** in return tx

## Whitehat self-spend OP_RETURNs (coordination)

- h=965818 [`c103de95817b43f2df635ec6f35ff126ca26a7c6d20570c4b01866b2b3e69a19`](https://mempool.space/tx/c103de95817b43f2df635ec6f35ff126ca26a7c6d20570c4b01866b2b3e69a19): we are whitehats. contact us on chain
- h=965869 [`3a3eac4a26395b8c2563aaf1eb8b1b77798c81c7d6337f51321827a244a480aa`](https://mempool.space/tx/3a3eac4a26395b8c2563aaf1eb8b1b77798c81c7d6337f51321827a244a480aa): sending most back to bc1qdlld6antmv4xug242ed83q7k4rqw50cwfns38szx4qu2f4jwaxxsuhwxxr, is that ok
- h=965875 [`83825b2135dd0abac12c9dfe17f29ab81b3427e1ae864947b0bebce5e47c3c4b`](https://mempool.space/tx/83825b2135dd0abac12c9dfe17f29ab81b3427e1ae864947b0bebce5e47c3c4b): Please fix the bug first. The chain is under risk at latest commit right now. Make sure every node is patched. Then we will transfer the money back safely after confirming the fix. The detail is as follows (encrypted using https://blockstream.com/pgp.txt).\n-----BEGIN PGP MESSAGE-----\n\nhQIMA7szLTHLpE7fAQ//e+6nCR6qz7BevVfT6JPweB0ZWoRkJsUxOb8AXLFDhEJh\nby7cnFBSLLlN9BK9g5PYIMDrRiYCIvDtDq2GquhaYKfRKVFWRqA8meH+/yOm3ZBK\n6j7lAXWrPjvvauDONI6avEqIORlsjmDZO4o2JcUlwX4u7Ovbc+ViqH5UrtSWB9N5\nM3yxXyve0QMSsECbSG7TJy6OJn1C8nX/eCFKg4VBY5b+VAGRvxq2nmJ4hmQrbU/n\nfdj4SuohMvV1l1Jt2Fc/SXCSZZQ3zDrWkDXWOUzYl+nvG68gTdSxO7i7PX5fIhiB\n7omMKvpEDngar9rBevryS4pDRwBr0rNoZSVAfKnHgfrf7PQpcGZeY7B7SMkWXaql\ns9zxBkvJ9Lol6BdRSCppq0KjPxOz3bSLzhRDvqrFY16uwqW7z+HYisY52e1grkcx\nCpmXfvZkfHErB9j+4emKLofXgCKd9qxJ3/awxAhrKF/7Njp+00HJ2inTshRskKSx\n0OP3lhWVyA29l4PKEv53r1hKPFlB0oxl81+frpkOtcA+gbpE2jtZweplR3RTTRlm\n532/DB0+T8CbSNgMKRuwlcMeGRFqFwB4cgsb8/gXHuA1I9dMp3XxBcohOQwkiJjY\n4GYP2wuH2dCj84NtAEXQfBOqSagmaE+7RZCGemxSNivkkaVd3cEOCfAxzGDsFOrS\nwJ4BYF6eB2Ty0NrTLaqcmYsdL8R2Q+5AxBohRapT5mlLLU7h3LXuzFcaE8EcyzKw\nQ4GRHG4UFMbMTIiy91+NL95Ox58NXyD50k/xH+FlSrLs1UM9HuaG2HEEYFfqvZN+\n/QjNLA0eVGRoLveAU5uqL39c5YE1VlgWCIaAl373GWZLhfWcnjhnGif/akngyzSX\n2S3VEaBIK3nDgi22CEMvgMGOQcfxPvn7l9MCUPh5Xdt2ihQzC8g4DY8XS2v9wCIl\n7hmwWUUe4/cm1lYb8mudP3Xdgi+3xVz8u7a4G2M23C0v/2pHQqQUEeCDCY+d2JzI\ntVT0LKmNZpCb72JaS+mzBE7TPpIfoftVMIp/FxcaEg79hP6EnpNAi0/oSjcaKroR\n3Ft/ALBPJVxhoABUBdtk7DurZ6pBjrHWVLlwXnW+8b2lDDzVE0M7s3RRdoZT+eHQ\nlVlBKPJ4pylW1PH+0rcmsg==\n=Xf9N\n-----END PGP MESSAGE-----
- h=965930 [`0256e33797ab173a5df1f2cd9689a81241f385dabb7e998e06ce78855ff19016`](https://mempool.space/tx/0256e33797ab173a5df1f2cd9689a81241f385dabb7e998e06ce78855ff19016): plz confirm again that we are sending the coins back to bc1qdlld6antmv4xug242ed83q7k4rqw50cwfns38szx4qu2f4jwaxxsuhwxxr\nMore details about the vuln fix:\n-----BEGIN PGP MESSAGE-----\n\nhQIMA7szLTHLpE7fARAArWiC95vahfVbVySvtqXMv4zj/xIII9fZ1Y4mAak51KwO\nkjt1OGVcnure5yStFIB3VMoiFeduD1T9O53ynyiLTpfNYVJd5isrUwUU3HuyaUdl\nOUeeUyHFfYKgvsQEtd\nOzgH77ceBYt/Y/69vME5142cUfEijAuSmtagfzZudjg8c4\n+vdiYS37toHBFli3Bb6iyL/HQ1QQPhGKEfKojKSjS0ONnpbsBNSRipfjCmwKMbbh\nHxqnLxRi8gq0OnSuamrx3C/sOB1jYaIOGFxEEml171eH87/Jluif/kLskZFEN5I+\ndrCAO8Y78Wn6/84uEp/pkI3fe8sqb4retX6ujaWMcJsHqeM8olJ1MNOOK20Zi2sL\n3Rt1J1YDapGN5XIYV8rTFdNPCVbLoFX1U5ANX3KyZdMEjRGOC5bmP+0fS6FuAfXZ\nXRRmTUTjmm74NKb4hJOfwWJnyE+ug3H6fU9CL4y11T3sSGanixLa5X5eL5BRU6vH\nqOJYtJulta/uTXWFgUofLgjJ8myCDf6W0RnsVIB+1bwQwS1KSdWsDxq8DhbvhTYT\nAKtFzkJwYnP7w5N/+pCEmy2QMmJq8rQMZSLqyEi8G9NcX8TOc+cKioWixPBVkiZV\ncGTF/qb5pp4qUf+7oQQ00afAhzxEf9gnC5Z2bS9HxEhdYLTqg+btSSxH2orK57nS\nwE8BBT4xBKFDNCeMoSbvq9djshRnWVMe3NjthvZyY9qnYuS+wVy0YB614gxVbWMT\nDeox7RPWlLOVMVTP92pUKQfGbmIUReU/G3fGidgK8/EyXjt0OWYsQPQ+YH7wlVq+\nA7KPjEoYPP35xezhIZNy5SkLrt3G9LG6ejBwclyx7TwGNGUw+/hbC6YNjTPgmZfD\n9Evhe9D+vh594UxHLQMqhqk6ZRug9FsBVA78hcGJlcbN7glBoPKLXAcWmrFyAGmZ\nDx45ZbhfVz8STEtlq33XTi1jsC5ConwVOqwAsdFjGvfT/tdgVHq/zJfYGcNnlXDZ\nrCo17QlGvz8IUms0z0TsmhwDksyoypRpZnzwSLu2F9cG\n=7BUN\n-----END PGP MESSAGE-----
- h=965948 [`4656114340749ba4aade1affcb2625a1dac37214b32b7d7c242af2ca9fe6d690`](https://mempool.space/tx/4656114340749ba4aade1affcb2625a1dac37214b32b7d7c242af2ca9fe6d690): -----BEGIN PGP MESSAGE-----\n\nhQIMA7szLTHLpE7fAQ//ZTmZBZV1mXdzTfOneFTZnduDZKUrRknPcer0qhv7Rd8F\nyMROKc0U+dDOOWpEdgtXC9XLhw5tSZaQF5vI9h45rrpQSogDwMlTZr5DFj4/ZOjz\nExC93CRXs/GJXTKWzdzomQRPiyWgcud0EY4PoT/l3Nuz0PUgImtoXU+Qk1PLg/ek\nWN4iRzCxGhLstk6IfzeZD68DBW+8rytESYWN1hgiYHgRxbYugcIYG6iUJdO0Eb/o\n6qwbbeGswHuJ3vW9Gt3h1AqJAaWScJLYCKipTFq8Rc8OvvTW8lPEp2GFaLM4tbro\nWAevCyoM9a1MrM3klt4xPYOymops9Cd9js5ueM+lqxcJZQpoUJ1XmQKVApszL86n\nNuLYzBCX3z/+JNLTCrfxenlUUQRTuNb/L18LXoctBtrvNAgWyo5+xolTHjhFCEAT\nJds6Tc0RVZt/jPwG0KQvOo1OFWEVM1iXp5rv9Xs5mzRYlhC37O4NANKYyUxqJ2NU\nDmpO4GYPlNGNOPLsCRAaKb6FpQzv1JMHNWxnC0sz9PaBjHAj3xI8kfbv8yzwmgL3\nSA36dT4p8pUScEUITH9/1xSPlfGlvRIlQm/7raiuv9u9ZvmI4d0kXkjJWUuwJl74\nDk7Uf2rfa+H6O3kAZl1/WY8oRUiWICbTKLkeFSux4mCiJvIUmZHwxmR6tKhVl/HS\nlAGBkFfo5dJnbsT77iLf5lwFR41p8JmmEAx+S2YDmiRnLYrLGIksO5AhWGDyixEv\n+hFc080BsY33TdzJBhmUdzXc3Dn92Fmi9zMc4dpsgwZrfHQ9heeEySm48CZuUgr3\n/2xlpEi4mU0yOuSf+gIsOfYiQ6dffBNBvviHZmsCrOaqNy3XTo7oLObuVLLONVnm\nizIxMp4=\n=UzNO\n-----END PGP MESSAGE-----

## Artifacts

- `op_returns.json` / `op_returns.csv` — full list
- `return_3400btc_tx.json` — return tx
- `txs_all.jsonl` — raw mempool.space txs

## Chronologie

Voir [`CHRONOLOGY.md`](CHRONOLOGY.md) et [`timeline.json`](timeline.json).
