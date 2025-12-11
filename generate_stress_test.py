#!/usr/bin/env python3
"""
Script de génération de maps de stress test pour R-Type
Génère des fichiers JSON avec un nombre massif d'entités

Usage:
    python3 generate_stress_test.py <entity_count> <output_file> [duration]

Examples:
    python3 generate_stress_test.py 1000 stress_1k.json 30
    python3 generate_stress_test.py 10000 stress_10k.json 60
    python3 generate_stress_test.py 100000 stress_100k.json 120
"""

import json
import random
import sys

def generate_stress_test_map(entity_count, output_file, spawn_duration=120.0):
    entities = []

    # Différents types d'ennemis disponibles
    enemy_types = [
        {
            "spritePath": "assets/sprites/r-typesheet5.png",
            "textureRect": [0, 0, 33, 36]
        },
        {
            "spritePath": "assets/sprites/r-typesheet9.png",
            "textureRect": [0, 0, 33, 36]
        },
        {
            "spritePath": "assets/sprites/r-typesheet10.png",
            "textureRect": [0, 0, 33, 36]
        },
        {
            "spritePath": "assets/sprites/r-typesheet11.png",
            "textureRect": [0, 0, 33, 36]
        }
    ]

    print(f"Generating {entity_count} entities...")

    for i in range(entity_count):
        if i % 1000 == 0 and i > 0:
            print(f"  Generated {i}/{entity_count} entities...")

        enemy_type = random.choice(enemy_types)

        entity = {
            "type": 1,
            "x": 1920,  # Spawn hors écran à droite
            "y": random.randint(50, 1030),  # Position Y aléatoire
            "spawnTime": round(random.uniform(0, spawn_duration), 5),
            "spritePath": enemy_type["spritePath"],
            "textureRect": enemy_type["textureRect"]
        }

        entities.append(entity)

    # Trier par spawnTime
    print("Sorting entities by spawn time...")
    entities.sort(key=lambda e: e["spawnTime"])

    # Créer le JSON final
    map_data = {
        "entities": entities
    }

    # Écrire dans le fichier
    print(f"Writing to {output_file}...")
    with open(output_file, 'w') as f:
        json.dump(map_data, f, indent=2)

    print("\n" + "="*60)
    print(f"✓ Successfully generated {entity_count} entities!")
    print(f"  Output file: {output_file}")
    print(f"  Spawn duration: {spawn_duration}s")
    print(f"  First spawn: {entities[0]['spawnTime']:.3f}s")
    print(f"  Last spawn: {entities[-1]['spawnTime']:.3f}s")
    avg_rate = entity_count / spawn_duration if spawn_duration > 0 else entity_count
    print(f"  Average spawn rate: {avg_rate:.1f} entities/second")
    print("="*60)

def generate_wave_pattern(entity_count, output_file, wave_count=10):
    """Génère des vagues d'ennemis espacées"""
    entities = []
    wave_duration = 120.0 / wave_count
    entities_per_wave = entity_count // wave_count

    enemy_types = [
        {"spritePath": "assets/sprites/r-typesheet5.png", "textureRect": [0, 0, 33, 36]},
        {"spritePath": "assets/sprites/r-typesheet9.png", "textureRect": [0, 0, 33, 36]},
        {"spritePath": "assets/sprites/r-typesheet10.png", "textureRect": [0, 0, 33, 36]}
    ]

    print(f"Generating {wave_count} waves with {entities_per_wave} entities each...")

    for wave in range(wave_count):
        wave_start = wave * wave_duration
        wave_end = wave_start + wave_duration * 0.5  # Vague dure la moitié du temps

        for i in range(entities_per_wave):
            enemy_type = random.choice(enemy_types)
            entity = {
                "type": 1,
                "x": 1920,
                "y": random.randint(100, 980),
                "spawnTime": round(random.uniform(wave_start, wave_end), 5),
                "spritePath": enemy_type["spritePath"],
                "textureRect": enemy_type["textureRect"]
            }
            entities.append(entity)

    entities.sort(key=lambda e: e["spawnTime"])

    map_data = {"entities": entities}

    with open(output_file, 'w') as f:
        json.dump(map_data, f, indent=2)

    print(f"\n✓ Generated {len(entities)} entities in {wave_count} waves")
    print(f"  Output file: {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(__doc__)
        print("\nQuick examples:")
        print("  # 1000 entities over 30 seconds")
        print("  python3 generate_stress_test.py 1000 stress_1k.json 30")
        print()
        print("  # 10000 entities over 60 seconds")
        print("  python3 generate_stress_test.py 10000 stress_10k.json 60")
        print()
        print("  # 5000 entities spawning almost simultaneously")
        print("  python3 generate_stress_test.py 5000 stress_simultaneous.json 0.5")
        sys.exit(1)

    entity_count = int(sys.argv[1])
    output_file = sys.argv[2]
    duration = float(sys.argv[3]) if len(sys.argv) > 3 else 120.0

    # Choix du pattern
    if "--waves" in sys.argv:
        wave_count = int(sys.argv[sys.argv.index("--waves") + 1]) if sys.argv.index("--waves") + 1 < len(sys.argv) else 10
        generate_wave_pattern(entity_count, output_file, wave_count)
    else:
        generate_stress_test_map(entity_count, output_file, duration)
