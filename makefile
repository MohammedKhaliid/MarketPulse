CXX = g++

all: producer consumer

producer: producer.cpp
	$(CXX) -o $@ $<

consumer: consumer.cpp
	$(CXX) -o $@ $<

clean:
	rm -f producer consumer