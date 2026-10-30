# SAÉ Système – Centre de tri de colis

Lucas TREIBER-LUSTIG · Tom SCHWARTZ · Marwan AIT TAMGOUNT

> L'arborescence et la répartition sont validées. Les valeurs de `commun.h` restent des **propositions** tant qu'on n'a pas fait le point ensemble (voir la section suivante).

**Compiler :** `make` · **Lancer :** `./centre_tri` · **Nettoyer :** `make clean`

---

## Sommaire

1. [⚠️ À fixer ensemble AVANT de coder](#️-à-fixer-ensemble-avant-de-coder)
2. [Où on en est](#où-on-en-est)
3. [Répartition des tâches](#répartition-des-tâches)
4. [Arborescence](#arborescence)
5. [Git : branches et règles](#git--branches-et-règles)

---

## ⚠️ À fixer ensemble AVANT de coder

> [!IMPORTANT]
> Chacun va coder une partie qui parle aux parties des autres. Si on ne se met pas d'accord **avant** sur ces points, rien ne marchera au moment de tout assembler. Tout ce qui est décidé ici va dans **`include/commun.h`**, et **personne ne le modifie seul** : on en parle d'abord sur le groupe.

Prévoir ~30 min tous les trois. Les valeurs ci-dessous sont des **propositions** à valider ou changer.

### ☐ 1. Le format d'un colis

| Question | Proposition |
|---|---|
| L'id est un nombre ou du texte ? | **Texte** (`char id[32]`). Le sujet dit juste « identifiant non vide », donc `COLIS abc A 2.5` devrait être accepté. |
| Type du poids | `float` |
| Où se fait la validation ? | Dans le thread **Tri & Routage** (c'est son rôle d'après le sujet). Le processus main vérifie seulement que la commande est bien formée. |

### ☐ 2. Le format des messages dans chaque pipe

On écrit une **struct en binaire** (`write(fd, &msg, sizeof msg)`) ou une **ligne de texte** ? Proposition : struct binaire, plus simple à relire de l'autre côté. Il faut la même struct des deux côtés de chaque pipe.

| Pipe | Type | De → Vers | Ce qui passe dedans |
|---|---|---|---|
| `fifo_commandes` | nommé | Processus main → Thread Main (TRI) | commandes `COLIS` / `ETAT` / `QUIT` |
| pipe tri | anonyme | Thread Main → Tri & Routage | colis à valider |
| pipe A / B / C | anonymes | Tri & Routage → Zone A / B / C | colis validés |
| `fifo_etat` | nommé | Supervision → Processus Affichage | état du centre + dernier colis traité |

### ☐ 3. Les FIFO (pipes nommés)

- Les noms/chemins exacts (ex. `/tmp/centre_tri_cmd` et `/tmp/centre_tri_etat`) → en `#define` dans `commun.h`.
- **Qui fait le `mkfifo`** (proposition : le processus main, avant les `fork`).
- **Qui fait le `unlink`** à la fin (proposition : le processus main aussi).
- Attention : `open` sur une FIFO bloque tant que l'autre côté n'est pas ouvert. Il faut se mettre d'accord sur l'ordre d'ouverture pour éviter un blocage au démarrage.

### ☐ 4. Les signaux : qui envoie quoi à qui

| Signal | De → Vers | Signification |
|---|---|---|
| `SIGUSR1` | Supervision → Thread Main | objectif de 20 colis atteint |
| `SIGTERM` | Thread Main → Processus main / Affichage | arrêt propre |
| `SIGUSR2` | TRI ↔ Affichage | synchro (ex. « nouvel état disponible ») |

À décider aussi :
- **Comment chaque processus connaît le PID des autres** (pour faire `kill`). Si c'est le processus main qui fait les deux `fork`, il a les deux PID mais TRI ne connaît pas celui d'Affichage. Solution simple : forker Affichage en premier puis passer son PID à TRI.
- **Signaux + threads** : dans un processus avec plusieurs threads, on ne sait pas quel thread reçoit le signal. Proposition : on bloque les signaux dans tous les threads (`pthread_sigmask`) et seul le thread Main les attend avec `sigwait`.

### ☐ 5. Communication Zones → Supervision (choix à justifier dans le rendu)

Le sujet nous laisse le choix mais **il faut le justifier**. Proposition :
- une **structure de stats partagée** (3 zones + global) protégée par un **mutex** ;
- une **variable de condition** que chaque zone signale après avoir traité un colis, pour réveiller Supervision au lieu de la faire tourner en boucle.

Justification : les threads partagent déjà la mémoire du processus, donc pas besoin de pipe en plus. Le mutex évite que deux zones écrivent en même temps, et la condition évite l'attente active.

### ☐ 6. La commande `ETAT`

Le sujet dit que seules les commandes `COLIS` passent dans le pipe nommé, mais ne dit pas clairement comment `ETAT` est traité. Proposition : la faire passer aussi dans `fifo_commandes`, et Supervision renvoie l'état courant à l'Affichage. **→ À confirmer avec M. Ayadi.**

### ☐ 7. L'arrêt propre (QUIT ou 20 colis)

C'est souvent là que ça plante, donc on fixe l'ordre dès maintenant :
1. Le thread Main reçoit `QUIT` ou le signal de Supervision.
2. Il ferme le pipe vers Tri & Routage → `read` renvoie 0 → Tri & Routage ferme les 3 pipes zones → les zones s'arrêtent en cascade.
3. `pthread_join` sur tous les threads.
4. Signal d'arrêt au processus main et à Affichage (Affichage ferme le journal proprement).
5. Le processus main fait `wait` sur ses fils et `unlink` les FIFO.

### ☐ 8. Conventions de code

- Compilation : `gcc -Wall -Wextra -pthread`, **0 warning** avant de pousser.
- Toujours tester le retour des appels système (`perror` + sortie si erreur grave).
- Commande invalide = message d'erreur et on continue, **jamais d'arrêt du programme** (exigé par le sujet).
- Lecture des commandes avec `fgets` + `sscanf`, pas `scanf` directement (voir plus bas pourquoi).
- Pas de variable globale à part celles qu'on a décidées ensemble (stats partagées, mutex).

---

## Où on en est

Premier commit de Lucas (26/10), déplacé dans `include/` et `src/` :

| Fichier | Contenu |
|---|---|
| `colis.h` / `colis.c` | struct `colis_t` (id, destination, poids) + `colis_init()` qui vérifie la destination et le poids |
| `zone.h` | struct `zone_t` (nb de colis, poids total), sans fonctions |
| `main.c` | 2 `fork()` (fils tri et affichage, vides pour l'instant) + boucle de lecture `COLIS` / `ETAT` / `QUIT`. Le colis est validé mais pas encore envoyé. |

En gros, la base de l'étape 1 est posée et ça compile proprement. Il n'y a encore ni pipe, ni thread, ni signal.

### Petits trucs à corriger

- [ ] **Commande invalide qui pollue la suite** : avec `COLIS abc A 2`, le `scanf` échoue et laisse `abc A 2` dans le buffer. Chaque mot est ensuite lu comme une commande, ce qui affiche 3 prompts en trop. → lire la ligne avec `fgets` puis `sscanf`.
- [ ] **`for(size_t i; ...)`** dans la boucle des `wait` : `i` n'est pas initialisé (comportement indéfini). → `size_t i = 0`.
- [ ] **L'id en `uint32_t`** refuse les id texte (voir point 1 plus haut).
- [ ] **`perror` sur une commande invalide** affiche « Invalid argument » en plus du message. → `fprintf(stderr, ...)`.
- [ ] Il manque un `\n` à la fin du message d'erreur de syntaxe.

---

## Répartition des tâches

| | Qui | Contenu |
|---|---|---|
| 🧭 **Pilotage** | **Lucas** | **Processus main** : lecture des commandes, `mkfifo`, `fork`, écriture dans `fifo_commandes`. **Thread Main du TRI** : création des 5 autres threads, lecture de `fifo_commandes`, pipe anonyme vers Tri & Routage, gestion de `QUIT` et des signaux, arrêt propre. |
| 📦 **Tri & zones** | **Marwan** | **Thread Tri & Routage** : validation des colis et envoi dans le bon pipe parmi les 3. **Threads Zone A / B / C** : lecture de leur pipe, mise à jour des stats de leur zone, envoi des infos à Supervision. |
| 📊 **Supervision & affichage** | **Tom** | **Thread Supervision** : stats globales, détection des 20 colis, écriture dans `fifo_etat`, signal au thread Main. **Processus Affichage** : affichage console, fichier journal, synchro par signaux. |

**Ce qu'on fait ensemble** : `commun.h`, le `Makefile`, la justification du choix Zones → Supervision et les tests finaux.

Points de contact entre les parties (là où il faut se parler) :
- Lucas ↔ Marwan : le pipe anonyme Thread Main → Tri & Routage.
- Marwan ↔ Tom : la structure partagée + le mutex/la condition entre les zones et Supervision.
- Tom ↔ Lucas : le signal « 20 colis atteints » et l'arrêt de l'Affichage.

---

## Arborescence

```
SAE-Systeme-Centre-de-tri-de-colis/
├── Makefile
├── README.md
├── .gitignore          # build/, exécutable, journal.log
├── include/
│   ├── commun.h        # FIFO, signaux, constantes (OBJECTIF 20), format des messages
│   ├── colis.h
│   └── zone.h
└── src/
    ├── main.c          # processus main                 (Lucas)
    ├── tri.c           # thread Main + création threads  (Lucas)
    ├── routage.c       # thread Tri & Routage            (Marwan)
    ├── zones.c         # threads Zone A/B/C              (Marwan)
    ├── supervision.c   # thread Supervision              (Tom)
    ├── affichage.c     # processus Affichage             (Tom)
    └── colis.c         # fonctions sur les colis
```

Pourquoi :
- **Un fichier par partie** : chacun travaille surtout dans ses propres fichiers, donc très peu de conflits git.
- **`include/` séparé de `src/`** : on voit tout de suite ce qui est partagé (les `.h`) et ce qui est interne.
- **Un `Makefile`** : tout le monde compile pareil avec `make`, plus de « chez moi ça marche ».
- **`.gitignore`** : pour ne pas pousser les exécutables ni le fichier journal.

---

## Git : branches et règles

### Une branche par fonctionnalité (pas par personne)

En entreprise, on ne fait presque jamais une branche par personne. On fait des **branches par fonctionnalité** (*feature branches*) qu'on fusionne dans `main` dès que la fonctionnalité marche, puis qu'on supprime. Ici, comme chacun a sa partie, ça revient presque au même, mais c'est plus propre et c'est une bonne habitude.

| Branche | Pour |
|---|---|
| `main` | uniquement du code qui compile et qui marche |
| `setup` | arborescence + `commun.h` + `Makefile` (fait en premier, fusionné vite) |
| `pilotage` | partie de Lucas |
| `zones` | partie de Marwan |
| `supervision` | partie de Tom |

Pour démarrer sa partie, une fois `setup` fusionnée :

```bash
git switch main
git pull
git switch -c zones
git push -u origin zones
```

Si une partie est grosse, on peut faire des petites branches dans la branche, ex. `zones-routage` puis `zones-threads`.

### Règles

1. **On ne pousse jamais directement sur `main`** : on passe par une Pull Request sur GitHub, et un autre membre jette un œil avant de fusionner.
2. **On récupère `main` souvent** dans sa branche (`git pull origin main`) pour éviter les gros conflits à la fin.
3. **`commun.h` ne se modifie pas en solo** : on prévient les autres, on fait une petite PR rien que pour ça.
4. **Ça compile sans warning** avant de pousser.
5. **Messages de commit clairs**, ex. `zones : mise à jour des stats sous mutex`.
