CLANGDIR=$PWD/prebuilts/clang/host/linux-x86
if [ ! -d "${CLANGDIR}/clang-r450784e" ]
then
    GREEN='\033[0;32m'
    NC='\033[0m'
    echo -e "${GREEN}Downloading clang 14.0.7${NC}"
    mkdir ${CLANGDIR}/clang-r450784e
    wget https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86/+archive/refs/heads/master/clang-r450784e.tar.gz -P ${CLANGDIR}
    tar -C "${CLANGDIR}"/clang-r450784e/ -zxf "${CLANGDIR}"/clang-r450784e.tar.gz
    rm ${CLANGDIR}/clang-r450784e.tar.gz
fi
