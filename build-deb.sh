#!/bin/sh

PACKAGE=libltc
DEBRELEASE=$(head -n1 debian/changelog | cut -d ' ' -f 2 | sed 's/[()]*//g')

TMPDIR=/tmp/$PACKAGE-${DEBRELEASE}
rm -rf ${TMPDIR}

GITBRANCH=${GITBRANCH:-master}

echo "debian -export-ignore" >> .git/info/attributes

git-buildpackage \
	--git-upstream-branch=$GITBRANCH --git-debian-branch=$GITBRANCH \
	--git-upstream-tree=branch \
	--git-export-dir=${TMPDIR} --git-cleaner=/bin/true \
	--git-force-create \
	-rfakeroot $@ 

ERROR=$?

ed -s .git/info/attributes > /dev/null << EOF
/debian -export-ignore
d
wq
EOF

if test $ERROR != 0; then
	exit $ERROR
fi

lintian -i --pedantic ${TMPDIR}/${PACKAGE}_${DEBRELEASE}_*.changes \
	| tee /tmp/${PACKAGE}.issues

echo
ls ${TMPDIR}/${PACKAGE}_${DEBRELEASE}_*.changes
echo
echo dput rg42 ${TMPDIR}/${PACKAGE}_${DEBRELEASE}_*.changes
