
"""Create and verify a complete Prism distribution from the upstream package."""

import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]

def sha256(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()

def imports(path):
    data = path.read_bytes()

    pe = struct.unpack_from("<I", data, 0x3C)[0]
    assert data[pe:pe + 4] == b"PE\0\0", path

    machine, count = struct.unpack_from("<HH", data, pe + 4)
    assert machine == 0x8664, f"Not x64: {path}"

    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    optional = pe + 24

    assert struct.unpack_from("<H", data, optional)[0] == 0x20B, path

    sections = []

    for i in range(count):
        offset = optional + optional_size + 40 * i

        virtual_size, virtual_address, raw_size, raw_offset = (
            struct.unpack_from("<IIII", data, offset + 8)
        )

        sections.append(
            (
                virtual_address,
                max(virtual_size, raw_size),
                raw_offset,
            )
        )

    def file_offset(rva):
        for address, size, raw in sections:
            if address <= rva < address + size:
                return raw + rva - address

        raise ValueError(f"Invalid RVA in {path}: {rva}")

    import_rva = struct.unpack_from("<I", data, optional + 112 + 8)[0]

    result = []

    if import_rva:
        cursor = file_offset(import_rva)

        while any(data[cursor:cursor + 20]):
            name_rva = struct.unpack_from("<I", data, cursor + 12)[0]
            name_offset = file_offset(name_rva)

            name_end = data.index(0, name_offset)

            result.append(
                data[name_offset:name_end].decode("ascii").lower()
            )

            cursor += 20

    return result

def main():
    build = Path(
        sys.argv[1] if len(sys.argv) > 1 else "build"
    ).resolve()

    # The CMake project name is DDNet, even though the executable is Prism.
    candidates = list(build.glob("DDNet-*-win64.zip"))

    assert len(candidates) == 1, (
        "Expected exactly one DDNet-*-win64.zip from package_default, "
        f"found: {candidates}"
    )

    commit = subprocess.check_output(
        ["git", "rev-parse", "HEAD"],
        cwd=ROOT,
        text=True,
    ).strip()

    expected = os.environ.get("PRISM_EXPECTED_COMMIT")

    if expected and commit != expected:
        raise RuntimeError(
            f"Package revision {commit} differs from "
            f"triggering revision {expected}"
        )

    dirty = subprocess.check_output(
        ["git", "status", "--porcelain", "--untracked-files=no"],
        cwd=ROOT,
        text=True,
    ).strip()

    if dirty:
        raise RuntimeError(
            "Refusing to attribute a dirty source tree to a clean commit"
        )

    tracked = subprocess.check_output(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
    ).decode().split("\0")

    sources = {
        name: sha256(ROOT / name)
        for name in tracked
        if name and (ROOT / name).is_file()
    }

    output = ROOT / "dist"
    output.mkdir(exist_ok=True)

    version_header = (
        ROOT / "src" / "game" / "client" / "prism_version.h"
    ).read_text(encoding="utf-8")

    version = version_header.split(
        'PRISM_VERSION "',
        1,
    )[1].split('"', 1)[0]

    release_version = os.environ.get("PRISM_RELEASE_VERSION")

    if release_version:
        assert version == release_version, (
            f"Release version {release_version} "
            f"differs from source version {version}"
        )

        filename = f"Prism-v{version}-windows-x64.zip"

    else:
        run_suffix = ""

        if "GITHUB_RUN_ID" in os.environ:
            run_suffix = (
                f"-{os.environ['GITHUB_RUN_ID']}"
                f"-{os.environ.get('GITHUB_RUN_ATTEMPT', '1')}"
            )

        filename = (
            f"prism-windows-x64-{version}-"
            f"{commit[:12]}{run_suffix}.zip"
        )

    artifact = output / filename

    if artifact.exists():
        raise FileExistsError(
            f"Refusing to overwrite existing artifact: {artifact}"
        )

    with tempfile.TemporaryDirectory() as temporary:
        temp = Path(temporary)

        with zipfile.ZipFile(candidates[0]) as archive:
            assert archive.testzip() is None, (
                "Upstream package ZIP CRC verification failed"
            )

            archive.extractall(temp)

        executables = list(temp.rglob("Prism.exe"))

        assert len(executables) == 1, (
            f"Expected one Prism.exe, found {executables}"
        )

        package = executables[0].parent

        # Include project documentation when the files exist.
        for name in [
            "README.md",
            "CHANGELOG.md",
            "CONTRIBUTING.md",
            "FOUNDATION.md",
            "VALIDATION.md",
        ]:
            source = ROOT / name

            if source.is_file():
                shutil.copy2(source, package / name)

        # Collect dependency license and copyright notices.
        for base in [
            ROOT / "data",
            ROOT / "src" / "engine" / "external",
            ROOT / "ddnet-libs",
        ]:
            if not base.exists():
                continue

            for path in base.rglob("*"):
                if not path.is_file():
                    continue

                if any(
                    word in path.name.lower()
                    for word in [
                        "license",
                        "licence",
                        "copying",
                        "copyright",
                        "notice",
                    ]
                ):
                    target = (
                        package
                        / "licenses"
                        / path.relative_to(ROOT)
                    )

                    target.parent.mkdir(
                        parents=True,
                        exist_ok=True,
                    )

                    shutil.copy2(path, target)

        info = {
            "project": "Prism",
            "version": version,
            "commit": commit,
            "upstream": "a5a61806e434ef22141b622db989087fbba8ed21",
            "source_sha256": sources,
            "configuration": "Release",
            "workflow_run": os.environ.get("GITHUB_RUN_ID"),
            "workflow_attempt": os.environ.get("GITHUB_RUN_ATTEMPT"),
            "submodules": subprocess.check_output(
                ["git", "submodule", "status", "--recursive"],
                cwd=ROOT,
                text=True,
            ).strip(),
        }

        (package / "BUILD_INFO.json").write_text(
            json.dumps(info, indent=2) + "\n",
            encoding="utf-8",
        )

        file_hashes = {
            str(path.relative_to(package)).replace("\\", "/"): sha256(path)
            for path in package.rglob("*")
            if path.is_file()
        }

        (package / "FILES_SHA256.json").write_text(
            json.dumps(file_hashes, indent=2) + "\n",
            encoding="utf-8",
        )

        with zipfile.ZipFile(
            artifact,
            "x",
            zipfile.ZIP_DEFLATED,
            compresslevel=6,
        ) as archive:
            for path in sorted(package.rglob("*")):
                if path.is_file():
                    archive.write(
                        path,
                        "Prism/" + path.relative_to(package).as_posix(),
                    )

    # Verify the final ZIP independently.
    with zipfile.ZipFile(artifact) as archive:
        assert archive.testzip() is None, (
            "Final ZIP CRC verification failed"
        )

        names = archive.namelist()

        required_files = [
            "Prism.exe",
            "SDL2.dll",
            "license.txt",
            "data/game.png",
            "BUILD_INFO.json",
            "FILES_SHA256.json",
        ]

        for name in required_files:
            assert "Prism/" + name in names, (
                f"Missing required package file: {name}"
            )

        assert any(
            name.startswith("Prism/data/fonts/")
            for name in names
        ), "Missing fonts"

        assert any(
            name.startswith("Prism/licenses/")
            for name in names
        ), "Missing dependency notices"

        build_info = json.loads(
            archive.read("Prism/BUILD_INFO.json")
        )

        assert build_info["commit"] == commit, (
            "Packaged commit does not match current commit"
        )

        hashes = json.loads(
            archive.read("Prism/FILES_SHA256.json")
        )

        for name, digest in hashes.items():
            actual = hashlib.sha256(
                archive.read("Prism/" + name)
            ).hexdigest()

            assert actual == digest, (
                f"File hash mismatch: {name}"
            )

        extracted = build / "verified-package"

        if extracted.exists():
            shutil.rmtree(extracted)

        archive.extractall(extracted)

    package = extracted / "Prism"

    binaries = [
        path
        for path in package.rglob("*")
        if path.suffix.lower() in [".dll", ".exe"]
    ]

    dlls = {
        path.name.lower()
        for path in binaries
    }

    resolved_imports = {}

    for binary in binaries:
        dependencies = imports(binary)
        resolved_imports[binary.name] = dependencies

        for dependency in dependencies:
            if dependency in dlls:
                continue

            if dependency.startswith(("api-ms-", "ext-ms-")):
                continue

            system_root = os.environ.get("SystemRoot")

            assert (
                os.name == "nt"
                and system_root
                and (
                    Path(system_root) / "System32" / dependency
                ).exists()
            ), (
                f"Missing dependency: "
                f"{binary.name} -> {dependency}"
            )

    digest = sha256(artifact)

    (output / (filename + ".sha256")).write_text(
        f"{digest}  {filename}\n",
        encoding="utf-8",
    )

    report = {
        "filename": filename,
        "commit": commit,
        "sha256": digest,
        "files": len(names),
        "zip_crc": "passed",
        "file_hashes": "passed",
        "x64_pe_imports": resolved_imports,
        "source_files": len(sources),
    }

    report_path = output / (
        filename[:-4] + "-verification.json"
    )

    report_path.write_text(
        json.dumps(report, indent=2),
        encoding="utf-8",
    )

    print(json.dumps(report, indent=2))

    if "GITHUB_OUTPUT" in os.environ:
        with open(
            os.environ["GITHUB_OUTPUT"],
            "a",
            encoding="utf-8",
        ) as stream:
            stream.write(
                f"artifact_name={filename[:-4]}\n"
            )

if __name__ == "__main__":
    main()
