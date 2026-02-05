.PHONY := all headers clean

CXX := g++-14
CXX_FLAGS := -std=c++20 -fmodules-ts -Wall -g
HEADER_FLAGS := -std=c++20 -fmodules-ts -c -x c++-system-header

EXEC := RAIInet
LIBS := -lncurses -lX11

OBJS := \
	types.o types-impl.o \
	board.o board-impl.o \
	link.o link-impl.o \
	errors.o errors-impl.o \
	ability.o ability-impl.o \
	player.o player-impl.o \
	cli.o cli-impl.o \
	game.o game-impl.o \
	window.o window-impl.o \
	view.o view-impl.o \
	controller.o controller-impl.o \
	raiinet.o

HEADERS := iostream fstream sstream exception memory string map vector cstdlib cctype

all: headers $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXX_FLAGS) $^ $(LIBS) -o $@
	
# --- Types ---
types.o: types.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

types-impl.o: types-impl.cc types.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- Board ---
board.o: board.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

board-impl.o: board-impl.cc board.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- Link ---
link.o: link.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

link-impl.o: link-impl.cc link.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- Ability ---
ability.o: ability.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

ability-impl.o: ability-impl.cc ability.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- Player ---
player.o: player.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

player-impl.o: player-impl.cc player.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- Game ---
game.o: game.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

game-impl.o: game-impl.cc game.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- X11 Window ---
window.o: window.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

window-impl.o: window-impl.cc window.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@


# --- View ---
view.o: view.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

view-impl.o: view-impl.cc view.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- Controller ---
controller.o: controller.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

controller-impl.o: controller-impl.cc controller.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- CLI ---
cli.o: cli.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

cli-impl.o: cli-impl.cc cli.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- Errors ---
errors.o: errors.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@

errors-impl.o: errors-impl.cc errors.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# --- raiinet main TU ---
raiinet.o: raiinet.cc \
	types.o types-impl.o \
	board.o board-impl.o \
	link.o link-impl.o \
	player.o player-impl.o \
	ability.o ability-impl.o \
	game.o game-impl.o \
	view.o view-impl.o \
	controller.o controller-impl.o \
	cli.o cli-impl.o \
	errors.o errors-impl.o
	$(CXX) $(CXX_FLAGS) -c $< -o $@

headers:
	$(CXX) $(HEADER_FLAGS) $(HEADERS)

clean:
	rm -rf ./gcm.cache $(EXEC) $(OBJS)
