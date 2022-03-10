CLANGDIR=$PWD/prebuilts/clang/host/linux-x86
if [ ! -d "${CLANGDIR}/clang-r445002" ]
then
    GREEN='\033[0;32m'
    NC='\033[0m'
    echo -e "${GREEN}Downloading clang 14.0.2${NC}"
    mkdir ${CLANGDIR}/clang-r445002
    wget https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86/+archive/refs/heads/master/clang-r445002.tar.gz -P ${CLANGDIR}
    tar -C "${CLANGDIR}"/clang-r445002/ -zxf "${CLANGDIR}"/clang-r445002.tar.gz
    rm ${CLANGDIR}/clang-r445002.tar.gz
fi
