import subprocess
import re
import sys

def get_last_tag():
    result = subprocess.run(['git', 'describe', '--tags', '--abbrev=0'], capture_output=True, text=True)
    return result.stdout.strip() if result.returncode == 0 else None

def get_commits_since_tag(tag):
    if tag:
        range_spec = f"{tag}..HEAD"
    else:
        range_spec = "HEAD"
    result = subprocess.run(['git', 'log', '--pretty=format:%s', range_spec], capture_output=True, text=True)
    return result.stdout.splitlines()

def categorize_commits(commits):
    features = []
    breaking = []
    bugfixes = []
    for msg in commits:
        if "BREAKING" in msg.upper():
            breaking.append(msg)
        elif re.search(r'\b(feature|add(ed)?)\b', msg, re.IGNORECASE):
            features.append(msg)
        elif re.search(r'\b(fix|bug)\b', msg, re.IGNORECASE):
            bugfixes.append(msg)
    return features, breaking, bugfixes

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

def main():
    last_tag = get_last_tag()
    commits = get_commits_since_tag(last_tag)
    features, breaking, bugfixes = categorize_commits(commits)
    next_version = bump_version(last_tag, breaking, features, bugfixes)

    print(f"# Release Notes (next version: v{next_version})\n")
    if breaking:
        print("## Breaking Changes")
        for msg in breaking:
            print(f"- {msg}")
    if features:
        print("\n## Features")
        for msg in features:
            print(f"- {msg}")
    if bugfixes:
        print("\n## Bugfixes")
        for msg in bugfixes:
            print(f"- {msg}")

    print("\n## Other Commits")
    others = set(commits) - set(features) - set(breaking) - set(bugfixes)
    for msg in others:
        print(f"- {msg}")

if __name__ == "__main__":
    main()
