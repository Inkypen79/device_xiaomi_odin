CLANGDIR=$PWD/prebuilts/clang/host/linux-x86
if [ ! -d "${CLANGDIR}/clang-r468909b" ]
then
    GREEN='\033[0;32m'
    NC='\033[0m'
    echo -e "${GREEN}Downloading clang 15.0.3${NC}"
    mkdir ${CLANGDIR}/clang-r468909b
    wget https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86/+archive/refs/heads/master/clang-r468909b.tar.gz -P ${CLANGDIR}
    tar -C "${CLANGDIR}"/clang-r468909b/ -zxf "${CLANGDIR}"/clang-r468909b.tar.gz
    rm ${CLANGDIR}/clang-r468909b.tar.gz
fi
