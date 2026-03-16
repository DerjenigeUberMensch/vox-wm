mkdir -p Release

make clean
make -j$(nproc) release
mv ./bin/release/vox-wm Release/vox-wmX86-64
cp Release/vox-wmX86-64 Release/vox-wm
make clean
make -j$(nproc) debug
mv ./bin/debug/vox-wm Release/vox-wmX86-64-debug

echo "Running Tests..."
echo "------------------"
./Release/vox-wm -v
echo "------------------"
echo "Make sure version is correct."

echo "Generated in -> Release/ Folder"
