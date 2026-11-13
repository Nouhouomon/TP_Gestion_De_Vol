# TP_Gestion_De_Vol
Gestion d'une agence de voyage (C++)
Presentation
Application console en C++ (programmation orientee objet) pour une agence
de voyage : gestion des passagers, des vols, des reservations, des
paiements et des annulations.
Fonctionnalites
Enregistrer un passager, ajouter un vol
Afficher les vols disponibles, rechercher un vol (numero, depart, destination)
Effectuer une reservation (EN ATTENTE), puis la confirmer par le paiement
Annuler une reservation avec remboursement selon le delai avant le depart
(>= 7 jours : 100 % | 3 a 6 jours : 50 % | moins de 3 jours : 0 %)
Consulter les reservations et le detail d'une reservation
Regles de gestion appliquees
Passager enregistre avant toute reservation
Reservations d'un vol <= capacite
Pas deux reservations actives du meme passager sur le meme vol
Une reservation = un passager + un vol
Une reservation annulee ne peut plus etre payee
Reservation definitive (CONFIRMEE) uniquement apres validation (paiement)
Paiement et annulation traites selon le statut de remboursement
Structure
Passager, Vol, Reservation, Paiement, Agence (+ classe utilitaire Date)
Encapsulation, constructeurs, associations par pointeurs, std::vector,
gestion des erreurs par exceptions.
Compilation et execution
g++ -std=c++17 -Wall -o agence main.cpp
./agence          (Linux / macOS)
agence.exe        (Windows)
Utilisation
Saisir le numero d'une option du menu (1 a 10) puis suivre les invites.
Les dates se saisissent au format JJ/MM/AAAA.
