# ICE Market Data & Execution Specs (placeholders)

Instructions:
1. Download ICE Market Data and Order Entry specification PDFs from ICE's Market Data / Technology portal (requires ICE account / market-data agreement).
2. Save the files in this directory using these suggested names:
   - ice-mdp-mbo-spec.pdf        (Market By Order)
   - ice-mdp-mbp-spec.pdf        (Market By Price)
   - ice-order-entry-fix-spec.pdf (FIX / Order Entry)
3. Commit them to the repo:
   - git add docs/ice-*.pdf
   - git commit -m "Add ICE market data/execution specs"
   - git push origin main

Notes:
- Some docs require explicit access; contact ICE support or your account rep if you cannot download them.
- If you want, I can create a small script to validate presence of these files before release.
