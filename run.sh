set -e

cmake .
make

echo "Running..."

mv ./asd ./example/asd
cd example
./asd $@