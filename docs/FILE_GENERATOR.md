# File generator: purpose and contract

The `file_generator` component produces synthetic market-data files used for testing and benchmarking.
Its intended contract:

- Produce binary messages that follow the exchange protocol layout exactly (field order, sizes, and binary encoding).
- Output files are byte-for-byte compatible with the corresponding live exchange feed packets for the supported message types.
- Files are suitable for offline ingestion by consumers that expect the real exchange feed.

Usage notes:
- Place generated files under `bin/<app>/<config>/` and consume them from tests or tools.
- If the exchange spec changes, update `src/apps/nsdq/itch/file_generator` to match and update tests accordingly.
- The release pipeline checks for the presence of generated binaries but does not assert semantic content; unit/integration tests should validate fields of interest.

Security / licensing:
- Do not include proprietary exchange specs in the repository unless your organization has rights to do so. Instead, keep specs in an internal docs area (see `docs/` for placeholders).
