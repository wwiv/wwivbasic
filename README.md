# wwivbasic (moved)

**wwivbasic now lives in the WWIV repository, under
[`wwivbasic/`](https://github.com/wwivbbs/wwiv/tree/main/wwivbasic).** This repository is
archived and no longer updated. Please open issues and pull requests against
[wwivbbs/wwiv](https://github.com/wwivbbs/wwiv).

wwivbasic is the ANTLR-based BASIC interpreter that is replacing my_basic as WWIV's scripting
engine. It was developed here through milestone M3 (the embedding API) and moved into WWIV at
the start of M4, since from then on most changes touch both the interpreter and the BBS. The
move kept the full history: the commits made here are in WWIV's history, and `git blame` on
files under `wwivbasic/` reaches them.

In WWIV:

- Build it with `-DWWIV_WWIVBASIC_NEXT=ON`; see
  [`wwivbasic/README.md`](https://github.com/wwivbbs/wwiv/blob/main/wwivbasic/README.md).
- The plan and milestones are in
  [`wwivbasic/docs/PLAN.md`](https://github.com/wwivbbs/wwiv/blob/main/wwivbasic/docs/PLAN.md),
  and the differences from my_basic in
  [`wwivbasic/PORTING.md`](https://github.com/wwivbbs/wwiv/blob/main/wwivbasic/PORTING.md).

The code in this repository is the state at the time of the move.
