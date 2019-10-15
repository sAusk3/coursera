package main

import (
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"sort"
)

func write(out io.Writer, file os.FileInfo, prefix string, isLast bool, isDir bool) error {
	var result string
	var size string
	if !isDir {
		sz := file.Size()
		if sz == 0 {
			size = " (empty)"
		} else {
			size = fmt.Sprintf(" (%db)", sz)
		}
	}
	if isLast {
		result = fmt.Sprintf("%s└───%s%s\n", prefix, file.Name(), size)
	} else {
		result = fmt.Sprintf("%s├───%s%s\n", prefix, file.Name(), size)
	}

	if _, err := out.Write([]byte(result)); err != nil {
		return err
	}
	return nil
}

func dirTreeHelp(out io.Writer, path string, printFiles bool, prefix string, isLast bool, isFirst bool) error {
	allFiles, err := ioutil.ReadDir(path)
	if err != nil {
		return err
	}

	files := make([]os.FileInfo, 0, len(allFiles))
	for _, file := range allFiles {
		if file.IsDir() {
			files = append(files, file)
		} else {
			if printFiles {
				files = append(files, file)
			}
		}
	}

	sort.Slice(files, func(i, j int) bool {
		if files[i].Name() < files[j].Name() {
			return true
		}
		return false
	})

	for i, file := range files {
		var newPrefix string
		if isFirst {
			newPrefix = ""
		} else if isLast {
			newPrefix = fmt.Sprintf("%s\t", prefix)
		} else {
			newPrefix = fmt.Sprintf("%s│\t", prefix)
		}

		if file.IsDir() {
			if err = write(out, file, newPrefix, i + 1 == len(files), true); err != nil {
				return err
			}

			if err = dirTreeHelp(out, fmt.Sprintf("%s/%s", path, file.Name()), printFiles, newPrefix, i + 1 == len(files), false); err != nil {
				return err
			}

		} else if printFiles {
			if err = write(out, file, newPrefix, i + 1 == len(files), false); err != nil {
				return err
			}
		}
	}

	return nil
}

func dirTree(out io.Writer, path string, printFiles bool) error {
	return dirTreeHelp(out, path, printFiles, "", false, true)
}

func main() {
	out := os.Stdout
	if !(len(os.Args) == 2 || len(os.Args) == 3) {
		panic("usage go run main.go . [-f]")
	}
	path := os.Args[1]
	printFiles := len(os.Args) == 3 && os.Args[2] == "-f"
	err := dirTree(out, path, printFiles)
	if err != nil {
		panic(err.Error())
	}
}
