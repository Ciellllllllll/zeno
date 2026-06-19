# SDK v0.1.0-rc.1 Release Publication Runbook

This runbook prepares the future publication process for `ZENO SDK v0.1.0-rc.1`.

Phase54 does not publish a GitHub Release, create a git tag, sign artifacts, upload assets to a release page, commit generated/downloaded SDK ZIPs, or add gameplay samples or engine features.

## Future Release Identity

- Release title: `ZENO SDK v0.1.0-rc.1`
- Future tag name: `sdk-v0.1.0-rc.1`
- Package label: `0.1.0-rc.1`
- Release artifact file: `ZenoEngine-SDK-v0.1.0-rc.1.zip`
- GitHub Actions artifact name: `ZenoEngine-SDK-v0.1.0-rc.1`

Do not create the tag until a later explicit release publication phase.

## Phase53 Artifact Provenance

- Source workflow: `SDK RC Artifact`
- Workflow input: `package_version=0.1.0-rc.1`
- Release gate result: Passed
- Debug headless tests in Actions: Passed
- Release headless tests in Actions: Passed
- SDK package consumption QA in Actions: Passed
- Downloaded Actions artifact: `ZenoEngine-SDK-v0.1.0-rc.1`
- Contained SDK ZIP: `ZenoEngine-SDK-v0.1.0-rc.1.zip`
- ZIP structure inspection: Passed
- Verified inner SDK ZIP SHA-256: `ebe50458edfb8f25f10d08fe8f13ad5681515147c986ffe6a7a164ad99369a29`

## Pre-Publish Checks

1. Confirm `git status --short` is clean.
2. Confirm the release commit contains the intended docs and workflow state.
3. Confirm no generated or downloaded SDK ZIP is staged.
4. Confirm no build output, executable, DLL, LIB, PDB, Cargo target output, or CMake build tree is staged.
5. Confirm no GitHub Release already exists for `sdk-v0.1.0-rc.1`.
6. Confirm no git tag named `sdk-v0.1.0-rc.1` already exists locally or remotely.
7. Confirm the downloaded Actions artifact is named `ZenoEngine-SDK-v0.1.0-rc.1`.
8. Confirm the artifact contains exactly the expected SDK ZIP file.
9. Confirm the SDK ZIP SHA-256 matches the Phase53 verified checksum.
10. Confirm the release draft text in `docs/release-draft-v0.1.0-rc.1.md` is still accurate.

## Future Tag Plan

In a later explicit release phase only:

```powershell
git tag sdk-v0.1.0-rc.1 <verified-release-commit>
git push origin sdk-v0.1.0-rc.1
```

The tag should point at the commit selected by the release owner after all pre-publish checks pass.

## Future Release Draft Plan

In a later explicit release phase only:

1. Open GitHub Releases.
2. Create a new draft release.
3. Set tag to `sdk-v0.1.0-rc.1`.
4. Set title to `ZENO SDK v0.1.0-rc.1`.
5. Copy the body from `docs/release-draft-v0.1.0-rc.1.md`.
6. Attach `ZenoEngine-SDK-v0.1.0-rc.1.zip`.
7. Verify the attached file name and checksum before publishing.
8. Publish only after release owner approval.

## Future Asset Upload Plan

Upload only this asset:

```text
ZenoEngine-SDK-v0.1.0-rc.1.zip
```

Do not upload:

- extracted SDK directories
- build directories
- Debug/Release executable outputs outside the ZIP
- downloaded Actions artifact wrapper archives
- logs
- PDB files
- generated local package folders
- duplicate ZIPs with different names

## Rollback And Correction

If the release is still a draft:

1. Remove the incorrect uploaded asset.
2. Correct the draft body or title.
3. Re-upload only the verified SDK ZIP if needed.
4. Re-check the displayed asset name and checksum.

If the release was accidentally published:

1. Do not overwrite the release silently.
2. Mark the release as a prerelease if that was missed.
3. Add a correction note to the release body.
4. If the uploaded asset is wrong, remove it and upload the verified asset.
5. If the tag points at the wrong commit, stop and decide in a separate explicit correction phase whether to delete/recreate the tag or publish a superseding RC.
6. Record the correction in a follow-up report.

If a bad artifact was downloaded by users:

1. Publish a visible correction note.
2. Prefer a superseding RC label such as `0.1.0-rc.2` if artifact integrity or provenance is uncertain.
3. Keep the old release notes clear about what changed.

## Post-Publish Checks

1. Confirm the release page title is `ZENO SDK v0.1.0-rc.1`.
2. Confirm the release tag is `sdk-v0.1.0-rc.1`.
3. Confirm the release is marked as a prerelease.
4. Download the published asset from the release page.
5. Confirm the downloaded file is `ZenoEngine-SDK-v0.1.0-rc.1.zip`.
6. Confirm SHA-256 is `ebe50458edfb8f25f10d08fe8f13ad5681515147c986ffe6a7a164ad99369a29`.
7. Extract the ZIP and confirm the top-level directory is `ZenoEngine-SDK-v0.1.0-rc.1/`.
8. Confirm `docs/release-notes.md`, `docs/release-checklist.md`, and SDK API/tutorial docs are present inside the package.
9. Confirm the release body includes known limitations and the Windows/MSVC/DirectX 11 scope.
10. Record the publication result in a later phase report.
