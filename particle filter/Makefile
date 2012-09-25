CC=g++
OPENCV= `pkg-config opencv --cflags --libs`
FLAGS = -std=c++98 -Wall -pedantic -w
bin = bin/
src = src/
dat = data/

all: $(bin)video_generator $(bin)tracker $(bin)libtest_particle

$(bin)video_generator: $(src)video_generator.cpp
	$(CC) $(FLAGS) $(src)video_generator.cpp -o $(bin)video_generator $(OPENCV)

$(bin)lbp.o: $(src)lbp.cpp $(src)lbp.h $(src)hist.h
	$(CC) $(FLAGS) -c $(src)lbp.cpp -o $(bin)lbp.o $(OPENCV)

$(bin)tracker.o: $(src)tracker.h $(src)tracker.cpp $(src)lbp.h
	$(CC) $(FLAGS) -c $(src)tracker.cpp -o $(bin)tracker.o $(OPENCV)

$(bin)location_area.o: $(src)location_area.h $(src)location_area.cpp
	$(CC) $(FLAGS) -c $(src)location_area.cpp -o $(bin)location_area.o $(OPENCV)

$(bin)object_stack.o: $(src)object_stack.h $(src)object_stack.cpp
	$(CC) $(FLAGS) -c $(src)object_stack.cpp -o $(bin)object_stack.o $(OPENCV)

$(bin)hist.o: $(src)hist.h $(src)hist.cpp
	$(CC) $(FLAGS) -c $(src)hist.cpp -o $(bin)hist.o

$(bin)particle.o: $(src)particle.h $(src)particle.cpp $(src)lbp.h $(src)hist.h
	$(CC) $(FLAGS) -c $(src)particle.cpp -o $(bin)particle.o 

$(bin)libtest_particle.o: $(src)libtest_particle.cpp
	$(CC) $(FLAGS) -c $(src)libtest_particle.cpp -o $(bin)libtest_particle.o

$(bin)libtest_particle: $(bin)libtest_particle.o $(bin)particle.o $(bin)hist.o $(bin)lbp.o
	$(CC) $(FLAGS) $(bin)libtest_particle.o $(bin)particle.o $(bin)hist.o $(bin)lbp.o -o $(bin)libtest_particle $(OPENCV)
	
$(bin)math.o: $(src)math.h $(src)math.cpp $(src)hist.h $(src)brief.h
	$(CC) $(FLAGS) -c $(src)math.cpp -o $(bin)math.o

$(bin)brief.o: $(src)brief.cpp $(src)brief.h
	$(CC) $(FLAGS) -c $(src)brief.cpp -o $(bin)brief.o

$(bin)main.o: $(src)main.cpp $(src)object_stack.h $(src)tracker.h
	$(CC) $(FLAGS) -c $(src)main.cpp -o $(bin)main.o $(OPENCV)

$(bin)tracker: $(bin)tracker.o $(bin)main.o $(bin)location_area.o $(bin)object_stack.o $(bin)hist.o $(bin)particle.o $(bin)lbp.o $(bin)brief.o $(bin)math.o
	$(CC) $(FLAGS) $(bin)tracker.o $(bin)main.o $(bin)location_area.o $(bin)object_stack.o $(bin)hist.o $(bin)particle.o $(bin)lbp.o -o $(bin)tracker $(OPENCV)

run:
	./$(bin)tracker -i $(dat)video.avi -o $(dat)tracked_video.avi

clean:
	rm -f $(bin)*

video:
	./$(bin)video_generator
