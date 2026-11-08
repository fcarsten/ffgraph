#!/usr/bin/env python3
"""Stage Release builds and create source/binary archives plus provenance."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tarfile
import tempfile
import zipfile


def git(source, *args):
    return subprocess.check_output(["git", "-C", str(source), *args])


def source_state(source, preview):
    dirty = bool(git(source, "status", "--porcelain", "--untracked-files=normal"))
    if dirty and not preview:
        raise ValueError("Release packaging requires a clean checkout; use --preview for uncommitted work")
    return git(source, "rev-parse", "HEAD").decode().strip(), "uncommitted-preview" if dirty else "clean"


def sha256(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def archive_source(source, target, root, preview):
    if not preview:
        # Archive Git objects, avoiding checkout line-ending transformations.
        subprocess.run(["git", "-C", str(source), "archive", "--format=tar.gz",
                        f"--prefix={root}/", f"--output={target}", "HEAD"], check=True)
        return
    names = git(source, "ls-files", "-z", "--cached", "--others", "--exclude-standard")
    with tarfile.open(target, "w:gz") as archive:
        for raw in sorted(set(names.split(b"\0")) - {b""}):
            name = raw.decode("utf-8")
            path = source / name
            if path.exists() or path.is_symlink():
                archive.add(path, arcname=f"{root}/{name}", recursive=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=Path("build/release-artifacts"))
    parser.add_argument("--preview", action="store_true", help="Explicitly label uncommitted packages as previews")
    args = parser.parse_args()
    source = Path(__file__).resolve().parents[1]
    build, output = args.build.resolve(), args.output.resolve()
    if output.is_relative_to(source):
        ignored = subprocess.run(["git", "-C", str(source), "check-ignore", "--quiet", str(output)])
        if ignored.returncode:
            raise ValueError("Output inside the checkout must be Git-ignored (for example build/release-artifacts)")
    version = (source / "VERSION").read_text().strip()
    if not re.fullmatch(r"\d+\.\d+\.\d+(?:-[A-Za-z0-9.]+)?", version):
        raise ValueError("Invalid VERSION")
    commit, state = source_state(source, args.preview)
    # Reconfigure/build so packaging cannot silently use a stale executable.
    subprocess.run(["cmake", "-S", str(source), "-B", str(build)], check=True)
    info = json.loads((build / "release-info.json").read_text())
    if info["version"] != version or info["build_type"] != "Release" or info["sanitizers"] != "OFF":
        raise ValueError("Use a matching Release build with sanitizers OFF")
    for name in ("bindir", "includedir", "libdir", "datadir", "docdir"):
        path = Path(info[name])
        if path.is_absolute() or ".." in path.parts:
            raise ValueError(f"Packaging needs relative installation directories: {name}")
    subprocess.run(["cmake", "--build", str(build), "--parallel", "2"], check=True)
    platform = f'{info["system"].lower()}-{info["architecture"]}'
    if info["system"] == "Windows" and info["architecture"] != "x86_64":
        raise ValueError("Windows release packaging currently supports x86-64 only")
    marker = "-preview" if args.preview else ""
    base = f"ffgraph-{version}{marker}"
    output.mkdir(parents=True, exist_ok=True)
    archives = []
    with tempfile.TemporaryDirectory(prefix="release-stage-", dir=build) as temporary:
        stage = Path(temporary) / base
        subprocess.run(["cmake", "--install", str(build), "--prefix", str(stage)], check=True)
        notices = stage / info["datadir"] / "licenses/ffgraph"
        for name in ("LICENSE", "container-COPYING", "legacy-tk-license.terms", "tcl.txt", "tk.txt",
                     "gcc-runtime.txt", "mingw.txt", "mingw-runtime.txt", "winpthreads.txt", "zlib.txt"):
            if not (notices / name).is_file():
                raise ValueError(f"Missing staged notice: {name}")
        files = {}
        for path in sorted(stage.rglob("*")):
            relative = path.relative_to(stage).as_posix()
            if path.is_symlink():
                files[relative] = {"symlink": str(path.readlink())}
            elif path.is_file():
                files[relative] = {"sha256": sha256(path)}
        manifest = {"release": version, "source_commit": commit, "source_state": state,
                    "preview": args.preview, "build": info, "files": files}
        manifest_text = json.dumps(manifest, indent=2, sort_keys=True) + "\n"
        (stage / "RELEASE-MANIFEST.json").write_text(manifest_text, encoding="utf-8")
        binary = output / f"{base}-{platform}"
        if info["system"] == "Windows":
            binary = Path(str(binary) + ".zip")
            with zipfile.ZipFile(binary, "w", compression=zipfile.ZIP_DEFLATED) as archive:
                for path in sorted(stage.rglob("*")):
                    if path.is_file():
                        archive.write(path, f"{base}/{path.relative_to(stage).as_posix()}")
        else:
            binary = Path(str(binary) + ".tar.gz")
            with tarfile.open(binary, "w:gz") as archive:
                archive.add(stage, arcname=base)
        archives.append(binary)
    source_archive = output / f"{base}-source.tar.gz"
    archive_source(source, source_archive, base, args.preview)
    archives.append(source_archive)
    manifest_path = output / f"{base}-{platform}-manifest.json"
    manifest_path.write_text(manifest_text, encoding="utf-8")
    archives.append(manifest_path)
    checksum_file = output / f"{base}-{platform}-SHA256SUMS"
    checksum_file.write_text("".join(f"{sha256(path)}  {path.name}\n" for path in archives), encoding="utf-8")
    print(f"Created {state} packages for {commit}:")
    for path in [*archives, checksum_file]:
        print(path)


if __name__ == "__main__":
    main()
