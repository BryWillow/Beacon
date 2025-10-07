CI / CD for Beacon
===================

This project uses GitHub Actions for CI and Release automation. The workflows live in `.github/workflows`.

Workflows
---------
- `ci.yml` — runs on push/PR to `main` and builds the project on both macOS and Ubuntu. Artifacts for `bin/Debug` are uploaded.
- `release.yml` — runs on semver tag pushes (e.g. `v1.2.3`) and builds a Release artifact, then creates a GitHub Release with the `bin/Release` contents.

Local development
-----------------
You can run the same build locally:

```bash
chmod +x ./build.sh
./build.sh debug -t:false
```

Secrets
-------
No secrets are required for CI to build. For the `release` workflow, GitHub Actions will use the repository's default GITHUB_TOKEN to create releases. If you require publishing to external registries, add secrets via repository Settings → Secrets.

Notes
-----
- Workflows intentionally keep test execution off by default (the build script supports flags to enable running tests).
- If you add new source files in new directories, ensure CMakeLists includes them (the build will fail in CI if CMake doesn't pick them up).
