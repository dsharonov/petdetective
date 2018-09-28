// task
package main

import (
	"bufio"
	"bytes"
	"fmt"
	"math"
	"os"
	"unicode"

	"github.com/dsharonov/petdetective/astar"
)

const maxCarBag = 4

const symCar = '@'
const symEdge = '+'
const symRoad = '*'

type house rune
type animal rune

func asAnimal(sym rune) animal {
	return animal(unicode.ToLower(sym))
}

func asHouse(sym rune) house {
	return house(unicode.ToUpper(sym))
}

func isHouse(sym rune) bool {
	return unicode.IsLetter(sym) && unicode.IsUpper(sym)
}

func isAnimal(sym rune) bool {
	return unicode.IsLetter(sym) && unicode.IsLower(sym)
}

func isRoad(sym rune) bool {
	return sym == symRoad
}

func isCar(sym rune) bool {
	return sym == symCar
}

type Task struct {
	graph   [][]rune
	carX    int
	carY    int
	carBag  []animal
	neighbs []*Task
}

func (t *Task) String() string {
	var buf bytes.Buffer

	buf.WriteString("  ")

	for i := 0; i < t.width(); i++ {
		buf.WriteString(fmt.Sprint(i % 10))
	}

	buf.WriteRune('\n')

	for i, row := range t.graph {
		buf.WriteString(fmt.Sprint(i%10, " "))
		buf.WriteString(string(row))
		buf.WriteRune('\n')
	}

	buf.WriteString(fmt.Sprintf("\nCar: (%d, %d)", t.carX, t.carY))
	buf.WriteString("\nIn car bag: ")

	for _, a := range t.carBag {
		buf.WriteRune(rune(a))
	}

	buf.WriteString(fmt.Sprintf("\nHeuristic: %f\n", t.heuristic()))

	return buf.String()
}

func (t *Task) key() string {
	var buf bytes.Buffer

	for _, row := range t.graph {
		buf.WriteString(string(row))
	}

	buf.WriteString(fmt.Sprintf("(%d, %d)", t.carX, t.carY))

	return buf.String()
}

// astar.Pather interface

func (t *Task) PathNeighbors() []astar.Pather {
	neighbours := make([]astar.Pather, 0, 4)

	for _, n := range t.neighbours() {
		neighbours = append(neighbours, n)
	}

	return neighbours
}

func (t *Task) PathNeighborCost(to astar.Pather) float64 {
	return 1.0
}

func (t *Task) PathEstimatedCost(to astar.Pather) float64 {
	return math.Abs(to.(*Task).heuristic() - t.heuristic())
}

func (t *Task) PathKey() string {
	return t.key()
}

////////////////////////////////////////////////

func (t *Task) isNil() bool {
	return t == nil || len(t.graph) == 0 || len(t.graph[0]) == 0
}

func (t *Task) width() int {
	if t.isNil() {
		return 0
	}

	return len(t.graph[0])
}

func (t *Task) height() int {
	if t.isNil() {
		return 0
	}

	return len(t.graph)
}

func (t *Task) checkBounds(x, y int) bool {
	return !t.isNil() && x >= 0 && x < t.width() && y >= 0 && y < t.height()
}

func (t *Task) heuristic() float64 {
	result := 0.0

	for _, row := range t.graph {
		for _, sym := range row {
			if isHouse(sym) || isAnimal(sym) {
				result += 1.0
			}
		}
	}

	result -= 0.5 * float64(len(t.carBag))

	return result
}

func (t *Task) get(x, y int) rune {
	return t.graph[y][x]
}

func (t *Task) set(x, y int, sym rune) {
	t.graph[y][x] = sym
}

func (t *Task) clone() *Task {
	if t.isNil() {
		return &Task{}
	}

	tcopy := &Task{carX: t.carX, carY: t.carY}

	// graph
	tcopy.graph = make([][]rune, 0, len(t.graph))
	for _, row := range t.graph {
		rowcopy := make([]rune, len(row))
		copy(rowcopy, row)
		tcopy.graph = append(tcopy.graph, rowcopy)
	}

	// carBag
	tcopy.carBag = make([]animal, len(t.carBag))
	copy(tcopy.carBag, t.carBag)

	// neighbours
	//	tcopy.neighbs := make([]*Task, len(t.neighbours))
	//	copy(tcopy.neighbs, t.neighbs)

	return tcopy
}

func (t *Task) hasRoomInCarBag() bool {
	return len(t.carBag) < maxCarBag
}

func (t *Task) isInCarBag(sym animal) bool {
	for _, s := range t.carBag {
		if s == sym {
			return true
		}
	}

	return false
}

func (t *Task) removeFromCarBag(sym animal) {
	i := 0
	for _, s := range t.carBag {
		if s != sym {
			t.carBag[i] = s
			i++
		}
	}

	t.carBag = t.carBag[:i]
}

func (t *Task) Target() *Task {
	target := t.clone()
	target.carBag = nil
	for y, row := range t.graph {
		for x, sym := range row {
			if isCar(sym) || isHouse(sym) || isAnimal(sym) {
				target.set(x, y, symRoad)
			}
		}
	}

	return target
}

func (t *Task) EqualsNoCar(other *Task) bool {
	if t.width() != other.width() || t.height() != other.height() {
		return false
	}

	// graph
	for x := 0; x < t.width(); x++ {
		for y := 0; y < t.height(); y++ {
			if t.get(x, y) != other.get(x, y) {
				return false
			}
		}
	}

	// car bag
	for _, s := range t.carBag {
		if !other.isInCarBag(s) {
			return false
		}
	}

	return true
}

func (t *Task) neighbours() []*Task {
	if t.neighbs != nil {
		return t.neighbs
	}

	type pair struct{ x, y int }

	nc := []pair{
		{t.carX - 2, t.carY},
		{t.carX + 2, t.carY},
		{t.carX, t.carY - 2},
		{t.carX, t.carY + 2},
	}

	ec := []pair{
		{t.carX - 1, t.carY},
		{t.carX + 1, t.carY},
		{t.carX, t.carY - 1},
		{t.carX, t.carY + 1},
	}

	for i := 0; i < len(nc); i++ {
		if !t.checkBounds(nc[i].x, nc[i].y) {
			continue
		}

		// no connection between nodes
		if t.get(ec[i].x, ec[i].y) != symEdge {
			continue
		}

		neigh := t.clone()
		// move car
		neigh.carX, neigh.carY = nc[i].x, nc[i].y
		neighsym := neigh.get(neigh.carX, neigh.carY)

		// change sym under car if needed
		switch {
		case isAnimal(neighsym) && neigh.hasRoomInCarBag():
			neigh.set(neigh.carX, neigh.carY, symRoad)
			neigh.carBag = append(neigh.carBag, asAnimal(neighsym))
		case isHouse(neighsym) && neigh.isInCarBag(asAnimal(neighsym)):
			neigh.set(neigh.carX, neigh.carY, symRoad)
			neigh.removeFromCarBag(asAnimal(neighsym))
		default:
		}

		t.neighbs = append(t.neighbs, neigh)
	}

	return t.neighbs
}

////////////////////////////////////////////////

func ReadTask(filename string) (*Task, error) {
	file, err := os.Open(filename)

	if err != nil {
		return nil, err
	}

	defer file.Close()

	task := &Task{carX: -1, carY: -1}

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		task.graph = append(task.graph, []rune(line))
	}

	if err := scanner.Err(); err != nil {
		return nil, err
	}

	if task.graph == nil {
		return nil, fmt.Errorf("Bad task file")
	}

	// find car
loop:
	for y, row := range task.graph {
		for x, sym := range row {
			if sym == symCar {
				task.carX, task.carY = x, y
				task.set(x, y, symRoad)
				break loop
			}
		}
	}

	if task.carX < 0 || task.carY < 0 {
		return nil, fmt.Errorf("Can't find car in task file")
	}

	return task, nil
}
