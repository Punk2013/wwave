CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lfftw3f -lm -I ./include

SOURCEDIR = src

TARGET = app

$(TARGET): $(wildcard $(SOURCEDIR)/*.cpp) $(wildcard $(SOURCEDIR)/*.c)
	$(CXX) $^ $(CFLAGS) $(LDFLAGS) -o $@

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

