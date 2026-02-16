CLANGDIR=$PWD/prebuilts/clang/host/linux-x86
CLANGVER=clang-r574158
if [ ! -d "${CLANGDIR}/${CLANGVER}" ]
then
    GREEN='\033[0;32m'
    NC='\033[0m'
    echo -e "${GREEN}Downloading clang 21.0.0${NC}"
    git clone https://gitlab.com/inkypen/prebuilts_clang_host_linux-x86 -b ${CLANGVER} "${CLANGDIR}/${CLANGVER}" --single-branch
fi

SEARCH_LINE="//device/google/cheets2/camera/v3"
INSERT_LINE="//device/xiaomi/odin/uwb"
cd $PWD/external/libchrome
if grep -qF "$INSERT_LINE" Android.bp; then
    croot
else
    sed -i "\|\"$SEARCH_LINE\"|a \        \"$INSERT_LINE\"," Android.bp
    croot
fi
