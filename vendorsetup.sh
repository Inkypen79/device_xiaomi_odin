CLANGDIR=$PWD/prebuilts/clang/host/linux-x86
CLANGVER=clang-r536225
if [ ! -d "${CLANGDIR}/${CLANGVER}" ]
then
    GREEN='\033[0;32m'
    NC='\033[0m'
    echo -e "${GREEN}Downloading clang 19.0.1${NC}"
    git clone https://gitlab.com/inkypen/prebuilts_clang_host_linux-x86 -b ${CLANGVER} "${CLANGDIR}/${CLANGVER}" --single-branch
fi

cd $PWD/hardware/nxp/uwb
git fetch https://android.googlesource.com/platform/hardware/nxp/uwb 0d0dd875c90ebc33e9a76c8397a7218742b51730 --quiet
git checkout FETCH_HEAD --quiet
croot
