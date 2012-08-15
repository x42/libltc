#!/bin/sh

EDITOR=${EDITOR:-editor}

if test ! -d .git; then
	echo "ERROR: This script runs only from a git repository."
	exit 1
fi

if test "`git symbolic-ref HEAD`" != "refs/heads/master"; then
	echo "ERROR: The git 'master' branch must be checked out."
	exit 1
fi

echo "commit pending changes.."
git commit -a 

# to bump the version number
# - src/ltc.h (this is the MAIN VERSION used by configure) 
#   and re-run autogen.sh to pick it up.
# - Changelog 
# - debian/changelog (for the debian package)

echo "Update version number -- edit 3 files: src/ltc.h ChangeLog debian/changelog"
echo -n "launch editor ? [Y/n]"
read -n1 a
echo

if test "$a" != "n" -a "$a" != "N"; then
	${EDITOR} src/ltc.h debian/changelog ChangeLog
	sh autogen.sh
fi

VERSION=$(awk '/ VERSION /{print $3;}' src/config.h | sed 's/"//g')

if [ -z "$VERSION" ]; then
  echo "unknown VERSION number"
  exit 1;
fi

echo "VERSION: $VERSION"

echo -n "Is this correct? [Y/n]"
read -n1 a
echo
if test "$a" == "n" -o "$a" == "N"; then
	exit 1
fi

echo "re-creating man-pages and documentation with new version-number.."
make dox 

echo "creating git-commit of updated doc & version number"
git commit -m "finalize changelog v${VERSION}" src/ltc.h ChangeLog debian/changelog doc/man/man3/ltc.h.3

cd doc/html
git add *.*
git commit -a -m "doxgen -- v${VERSION}"
cd ../..

git tag "v${VERSION}" || (echo -n "version tagging failed. - press Enter to continue, CTRL-C to stop."; read; ) 

echo -n "git push? [Y/n]"
read -n1 a
echo

if test "$a" != "n" -a "$a" != "N"; then
	git push origin || exit 1
	#git push --tags ## would push ALL existing tags,
	git push origin "refs/tags/v${VERSION}:refs/tags/v${VERSION}" || exit 1
	cd doc/html
	git push origin gh-pages || exit 1
	cd ../..
fi

make dist
ls -l "libltc-${VERSION}.tar.gz"

GITREPO="x42/libltc"
x-www-browser https://github.com/${GITREPO}/downloads
