package main

import (
	"fmt"
	"os"

	"github.com/dsharonov/astar"
)

//func WaitInput() {
//	reader := bufio.NewReader(os.Stdin)
//	reader.ReadString('\n')
//}

func main() {

	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <task_filename>\n", os.Args[0])
		os.Exit(1)
	}

	for narg := 1; narg < len(os.Args); narg++ {
		taskFilename := os.Args[narg]

		fmt.Println("=======================================")
		fmt.Printf("Processing %s\n\n", taskFilename)

		task, err := ReadTask(taskFilename)
		if err != nil {
			fmt.Printf("Error: %v\n", err)
			continue
		}

		target := task.Target()

		path, _, found := astar.Path(task, target)
		if !found {
			fmt.Println("Could not find path")
			continue
		}

		numsteps := 0
		for i := len(path) - 1; i >= 0; numsteps, i = numsteps+1, i-1 {
			fmt.Printf("--- %d ---\n", int(len(path)-i-1))
			fmt.Println(path[i])
			if target.EqualsNoCar(path[i].(*Task)) {
				break
			}
		}

		fmt.Printf("Target reached in %d steps\n", numsteps)
	}

}

