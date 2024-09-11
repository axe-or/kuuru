#!/usr/bin/env sh

set -eu

archive="kuuru-$(git rev-parse --short=8 HEAD).zip"
git archive HEAD --format=zip > "$archive"
zip -r "$archive" base
echo "Created $archive"
