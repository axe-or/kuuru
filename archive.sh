#!/usr/bin/env sh

git archive HEAD --format=zip > "yuyu-$(git rev-parse --short=8 HEAD).zip"
