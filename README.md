# Genetic_Architect
Genetic algorithm that should produce floor plans and arrangements. Probably will never work properly.

## How it works

The main idea is
1. generate random *genomes* (generation)
2. score them according to *soft constraints* (fitness)
3. choose *genomes* with probability proportional to its scores
4. combine chosen genomes into a new population (crossover)
5. mutate some *genes* (mutation)
6. repeat from 2

## Compile
```bash
g++ -o ga.o main.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs
```

## Glossary
- Genome: The representation of the floor plan in memory. Consists of many *genes*
- Gene: The representation of a space(room/walkway etc) in the *genome*.
- Score: How good a *genome* is according to the soft constraints. Bigger is better (or so I've heard).
- Soft constraints: The things that are used to score a *genome*. For example
  - Distance between room and bathroom
  - Distance between kitchen and living room
  - Wasted area for walkways
  - etc.
- Hard constraints: The things that make a *genome* acceptable or not. Failing to pass a hard constraint implies a *score* of 0.
  - Plot limits
  - Access to rooms
  - Minimum dimensions

## Known bugs
- haha yes

## Requirements
OpenCV > 2.0
