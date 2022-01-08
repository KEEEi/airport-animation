TARGET = Project\2
CXXFLAGS = -O2	#-g
LDFLAGS = -lncurses -lpthread

default: $(TARGET)

#Ceates the executable binary
$(TARGET): AirportAnimator.cpp planeTest.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:#clean planes
	rm -f $(TARGET) *.o *~
