# Release Notes

## Release Version: `v1.2.3.4`
**Release Date**: 2025-10-16  
**Author**: Bryan Camp  

---

### Summary
This release includes updates to the build pipeline, optimizations for the file generator, and various bug fixes.

---

### Changes
#### New Features
- **Build Pipeline Enhancements**: Improved modularity and added support for app-specific builds.
- **Release Notes Automation**: Automated generation of release notes based on Git history.

#### Bug Fixes
- **File Generator**: Fixed an issue with out-of-source builds not resolving dependencies correctly.
- **Build Script**: Resolved a bug where the build number was incremented on failed builds.

#### Improvements
- **Code Style**: Enforced consistent formatting across all scripts.
- **Documentation**: Updated README with instructions for checking out specific versions.




---

### Version History

| Version   | Date       | Hash     | Message                          | Author          |
|-----------|------------|----------|----------------------------------|-----------------|
| v1.2.3.4  | 2025-10-16 | cccd998  | Updating release script          | Bryan Camp      |
| v1.2.3.4  | 2025-10-16 | 13cb37b  | Updated release script           | Bryan Camp      |
| v1.2.3.4  | 2025-10-15 | c18f67d  | Updated version number           | Bryan Camp      |
| v1.2.3.4  | 2025-10-15 | 261a82e  | Added release notes              | Bryan Camp      |
| v1.2.3.3  | 2025-10-14 | b73b2d6  | Updated top-level build scripts  | Bryan Camp      |
| v1.2.3.3  | 2025-10-13 | 6b8711b  | Complete directory restructure   | Bryan Camp      |
| v1.2.3.3  | 2025-10-13 | 325b1b1  | Added out-of-source build script | Bryan Camp      |

---

### Build Artifacts

After running the `build.sh` script, the executables for each application will be located in the following directories:

```plaintext
bin/<app_name>/<mode>/
```

- `<app_name>`: The name of the application (e.g., `file_generator`, `udp_replayer`, `matching_engine`).
- `<mode>`: The build mode, which can be `debug` or `release`.

#### Example:
- For the `file_generator` application built in `debug` mode:
  ```plaintext
  bin/file_generator/debug/
  ```
- For the `udp_replayer` application built in `release` mode:
  ```plaintext
  bin/udp_replayer/release/
  ```

---

### Checking Out a Specific Version

To check out a specific version of the Beacon project, use the following Git command:

```bash
git checkout tags/v1.2.3.4
```

This will give you a read-only view of the source code for that version. To return to the main branch, run:

```bash
git checkout main
```