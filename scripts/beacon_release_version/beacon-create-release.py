import subprocess
import re
from datetime import datetime

def get_last_tag():
    result = subprocess.run(['git', 'describe', '--tags', '--abbrev=0'], capture_output=True, text=True)
    return result.stdout.strip() if result.returncode == 0 else None

def get_commits_since_tag(tag):
    if tag:
        range_spec = f"{tag}..HEAD"
    else:
        range_spec = "HEAD"
    # Format: hash|date|message
    git_format = "%h|%ad|%s"
    result = subprocess.run([
        'git', 'log', '--pretty=format:' + git_format, '--date=short', range_spec
    ], capture_output=True, text=True)
    return [line.split('|', 2) for line in result.stdout.splitlines()]

def categorize_commits(commits):
    features = []
    breaking = []
    bugfixes = []
    others = []
    for hash_, date, msg in commits:
        entry = {"hash": hash_, "date": date, "msg": msg}
        if "BREAKING" in msg.upper():
            breaking.append(entry)
        elif re.search(r'\b(feature|add(ed)?)\b', msg, re.IGNORECASE):
            features.append(entry)
        elif re.search(r'\b(fix|bug)\b', msg, re.IGNORECASE):
            bugfixes.append(entry)
        else:
            others.append(entry)
    return features, breaking, bugfixes, others

def bump_version(last_tag, breaking, features, bugfixes):
    if last_tag and re.match(r'v?(\d+)\.(\d+)\.(\d+)', last_tag):
        major, minor, revision = map(int, re.findall(r'\d+', last_tag))
    else:
        major, minor, revision = 1, 0, 0

    # Never bump major
    if breaking or features:
        minor += 1
        revision = 0
    elif bugfixes:
        revision += 1
    # else, no bump

    return f"{major}.{minor}.{revision}"

def pretty_entry(entry):
    return f"- `{entry['date']}` [`{entry['hash']}`] {entry['msg']}"

def tag_repo(version):
    tag_name = f"v{version}"
    # Create annotated tag
    subprocess.run(['git', 'tag', '-a', tag_name, '-m', f"Release {tag_name}"])
    # Push tag to origin main
    subprocess.run(['git', 'push', 'origin', tag_name])
    print(f"\nTagged repository with {tag_name} and pushed to origin.")

def main():
    last_tag = get_last_tag()
    commits = get_commits_since_tag(last_tag)
    if not commits:
        print("No new commits since the previous release. Aborting release note and tagging process.")
        return

    features, breaking, bugfixes, others = categorize_commits(commits)
    next_version = bump_version(last_tag, breaking, features, bugfixes)

    print(f"# Release Notes (next version: v{next_version})\n")
    if breaking:
        print("## Breaking Changes")
        for entry in breaking:
            print(pretty_entry(entry))
    if features:
        print("\n## Features")
        for entry in features:
            print(pretty_entry(entry))
    if bugfixes:
        print("\n## Bugfixes")
        for entry in bugfixes:
            print(pretty_entry(entry))
    if others:
        print("\n## Other Commits")
        for entry in others:
            print(pretty_entry(entry))

    # Prompt for tagging
    confirm = input(f"\nTag main branch with version v{next_version}? [y/N]: ").strip().lower()
    if confirm == 'y':
        tag_repo(next_version)
    else:
        print("Tagging skipped.")

if __name__ == "__main__":
    main()
