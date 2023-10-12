if cmake -S . -B build; then
	cmake --build build
	if [[ "$OSTYPE" == "darwin"* ]]; then
	  ./build/pong
	elif [[ "$OSTYPE" == "msys" ]]; then
	  ./build/pong.exe
	fi
fi
