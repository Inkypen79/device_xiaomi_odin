CLANGDIR=$PWD/prebuilts/clang/host/linux-x86
if [ ! -d "${CLANGDIR}/clang-r458507" ]
then
    GREEN='\033[0;32m'
    NC='\033[0m'
    echo -e "${GREEN}Downloading clang 15.0.1${NC}"
    mkdir ${CLANGDIR}/clang-r458507
    wget https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86/+archive/refs/heads/master/clang-r458507.tar.gz -P ${CLANGDIR}
    tar -C "${CLANGDIR}"/clang-r458507/ -zxf "${CLANGDIR}"/clang-r458507.tar.gz
    rm ${CLANGDIR}/clang-r458507.tar.gz
fi
