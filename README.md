# 🛸 R-Type — Online Multiplayer Game (C++ / Asio / ECS)

## Présentation du projet

**R-Type** est un projet réalisé dans le cadre du module **Advanced C++ / Network Programming** à **Epitech Technology**.  
L’objectif est de **recréer un jeu multijoueur inspiré du classique R-Type**, en mettant en œuvre :
- Une **architecture client / serveur** en **C++17** (ou supérieur)
- Une communication réseau en **UDP** (via **Asio**)
- Un **moteur de jeu modulaire** basé sur un **ECS (Entity Component System)**

Le jeu permet à plusieurs joueurs de se connecter, de se déplacer, de tirer et d’affronter des vagues d’ennemis en temps réel.

---

## Fonctionnalités principales

### Côté Joueur (Client)
- Connexion au serveur (JOIN)
- Gestion des inputs clavier
- Envoi des actions en temps réel via UDP
- Affichage du monde de jeu et des entités (vaisseaux, tirs, ennemis)
- Synchronisation des positions via snapshots réseau

### Côté Serveur
- Gestion de la boucle réseau Asio (async)
- Réception et traitement des paquets clients
- Attribution dynamique d’un PlayerID
- Broadcast des événements de jeu à tous les clients
- Système de slots de joueurs (max. 4)
- Gestion des événements : spawn, déplacement, tir, collision, mort

### Architecture interne
- **ECS (Entity Component System)** pour une gestion modulaire du gameplay
- **Thread principal réseau** asynchrone (Asio)
- **Système de sérialisation binaire** pour les paquets réseau
- **Gestion des timestamps et packetIds** pour la synchronisation

---

## Structure du projet

R-Type/
├── CMakeLists.txt
├── README.md
├── docs/
│ ├── architecture.md
│ ├── protocol.md
│ ├── comparative-study.md
│ └── accessibility.md
├── src/
│ ├── client/
│ │ ├── main.cpp
│ │ └── ...
│ ├── server/
│ │ ├── NetworkServer.cpp
│ │ └── ...
│ └── engine/
│ ├── ecs/
│ └── components/
├── assets/
│ └── sprites/
└── tests/

---

## ⚙️ Installation et compilation

### Dépendances
Assure-toi d’avoir :
- **CMake ≥ 3.20**
- **C++17** (ou supérieur)
- **Asio** (ou Boost.Asio)
- **SFML** ou **Raylib** (selon ton moteur de rendu)
- **Conan** ou **Vcpkg** (facultatif, pour la gestion des libs)

### Compilation

#### 1. Cloner le projet :
```bash
git clone https://github.com/EpitechPGE3-2025/G-CPP-500-BDX-5-1-rtype-7.git
cd G-CPP-500-BDX-5-1-rtype-7
```

#### 2. Compiler et lancer le serveur :

```bash
cd server
./build.sh
./r-type_server -h [port] -p [port]
```

#### 3. Compiler et lancer le client :

```bash
cd client
./build.sh
./r-type_client -h [port] -p [port]
```

### 🔌 Communication réseau

Le protocole est basé sur UDP avec une structure binaire fixe :

```
[Type:1][PacketID:2][Timestamp:4][Payload:n]
```

Exemples de paquets :

| Type | Nom                  | Description                          |
| ---- | -------------------- | ------------------------------------ |
| 0x01 | JOIN                 | Un joueur rejoint la partie          |
| 0x02 | INPUT                | Action clavier envoyée               |
| 0x03 | PING                 | Vérification de latence              |
| 0x04 | SNAPSHOT             | Synchronisation du monde             |
| 0x05 | PLAYER_EVENT         | Événements liés aux joueurs          |
| 0x06 | ENTITY_EVENT         | Événements liés aux entités          |
| 0x07 | PLAYER_ID_ASSIGNMENT | Attribution d’un ID unique au joueur |


### 🧩 Architecture technique

🔹 Côté Serveur

NetworkServer : gère la réception et l’envoi de paquets

PlayerSlot : structure d’un joueur connecté

PacketHandler : logique de traitement (JOIN, INPUT, etc.)

GameWorld : logique de jeu et synchronisation

🔹 Côté Client

NetworkClient : gère la communication avec le serveur

Renderer : affichage du jeu

InputManager : lecture des événements clavier

ECS : gestion des entités et composants

### 👥 Équipe

| Nom               | Rôle                             |
| ----------------- | -------------------------------- |
| **Antton Ducos** | Stagiaire ECS / branleur |
| **Louka Ortega-cand** | Stagiaire ECS / Kassos |
| **Rémy Thai** | Développeur client / Interface de jeu |
| **Simon Maigrot** | Développeur réseau / Serveur UDP |

### 🧪 Tests
Les tests unitaires peuvent être exécutés avec :
```
cd build
ctest
```

Ils couvrent :

La sérialisation des paquets

Les interactions client/serveur

Les composants ECS

### 🧭 Ressources techniques

Documentation Asio : https://think-async.com

ECS Pattern : https://skypjack.github.io/entt/

UDP Game Networking : Valve Developer Wiki

SFML / Raylib Docs : https://www.sfml-dev.org
 / https://www.raylib.com

### ⚖️ Licence

Projet développé dans le cadre pédagogique d’Epitech.
Usage réservé à des fins d’apprentissage et de démonstration technique.
