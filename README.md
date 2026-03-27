# Conception et mise œuvre MultiCore Monitor, un moniteur temps réel de performance multiprocesseur: cas de Linux openSUSE
Ce projet implémente un moniteur de processus multi-cœur. Il se compose d'un module noyau Linux (LKM) qui extrait les données des processus, d'un serveur de contrôle en C, et d'une interface web pour la visualisation.

# Objectifs:
Visualiser la répartition des charges et la performance sur plusieurs cœurs de CPU.
 
# Fonctionnalités attendues
Tableau de bord web (temps réel) de répartition des charges et la performance des cœurs de CPU.
Gestion de l'affinité CPU.
Extraction des données en temps réel.
Cartographie par coeur.
Identification des taches.

#Architecture
Noyau (core_monitor_lkm.c) : Explore la task_struct pour lister les processus par processeur.
Backend (core_manager.c) : Serveur HTTP (libmicrohttpd) qui expose les données JSON et gère l'affinité.
Frontend (index.html, js, css) : Interface utilisateur pour afficher les statistiques en temps réel.

#Prérequis sur openSUSE
Mettre les fichiers (code source) dans dans meme dossier
Installer les outils de compilation et les bibliothèques nécessaires :
bash
sudo zypper install devel_kernel devel_C_C++ libmicrohttpd-devel

#Installation et Lancement

1. Compiler et charger le module
Le script lancement.sh automatise ces étapes :
bash
chmod +x lancement.sh
./lancement.sh

2. Accéder à l'interface
Une fois le serveur lancé, ouvrir le fichier index.html dans le navigateur ou accéder l'adresse configurée :
URL API : http://localhost:8080 (données JSON)
Interface : http://localhost:8080/index.html .

# NB: README à mettre à jour progressivement par l'équipe.
