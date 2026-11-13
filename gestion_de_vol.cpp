// ============================================================
//  Gestion d'une agence de voyage - Application console C++
//  Passagers, Vols, Reservations, Paiements et Annulations
//  Compilation : g++ -std=c++17 -Wall -o agence main.cpp
// ============================================================
#include <algorithm>
#include <cctype>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// ------------------------------------------------------------
//  Classe Date (jour/mois/annee)
// ------------------------------------------------------------
class Date {
private:
    int jour, mois, annee;

public:
    Date(int j = 1, int m = 1, int a = 2026) : jour(j), mois(m), annee(a) {}

    static bool estValide(int j, int m, int a) {
        if (a < 2000 || a > 2100 || m < 1 || m > 12 || j < 1) return false;
        static const int jm[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (j > jm[m - 1]) return false;
        bool bissextile = (a % 4 == 0 && a % 100 != 0) || (a % 400 == 0);
        if (m == 2 && j == 29 && !bissextile) return false;
        return true;
    }

    // Lit une date au format JJ/MM/AAAA
    static bool lire(const string& s, Date& resultat) {
        int j, m, a;
        char c1, c2;
        istringstream iss(s);
        if (!(iss >> j >> c1 >> m >> c2 >> a) || c1 != '/' || c2 != '/') return false;
        string reste;
        if (iss >> reste) return false;
        if (!estValide(j, m, a)) return false;
        resultat = Date(j, m, a);
        return true;
    }

    string toString() const {
        ostringstream oss;
        oss << setfill('0') << setw(2) << jour << "/" << setw(2) << mois << "/" << annee;
        return oss.str();
    }

    // Nombre de jours entre aujourd'hui et cette date (negatif si passee)
    int joursRestants() const {
        tm t = {};
        t.tm_mday = jour;
        t.tm_mon = mois - 1;
        t.tm_year = annee - 1900;
        t.tm_hour = 12;
        time_t depart = mktime(&t);
        return static_cast<int>(floor(difftime(depart, time(nullptr)) / 86400.0));
    }
};

// ------------------------------------------------------------
//  Classe Passager
// ------------------------------------------------------------
class Passager {
private:
    int id;
    string nom, prenom, telephone;

public:
    Passager(int id, const string& nom, const string& prenom, const string& telephone)
        : id(id), nom(nom), prenom(prenom), telephone(telephone) {}

    int getId() const { return id; }
    const string& getNom() const { return nom; }
    const string& getPrenom() const { return prenom; }
    const string& getTelephone() const { return telephone; }
    string getNomComplet() const { return prenom + " " + nom; }

    void afficher() const {
        cout << "  Passager #" << id << " : " << getNomComplet()
             << " | Tel : " << telephone << endl;
    }
};

// ------------------------------------------------------------
//  Classe Vol
// ------------------------------------------------------------
class Vol {
private:
    string numero, depart, destination, heure;
    Date date;
    int capacite;
    int placesReservees;
    double prix;

public:
    Vol(const string& numero, const string& depart, const string& destination,
        const Date& date, const string& heure, int capacite, double prix)
        : numero(numero), depart(depart), destination(destination), heure(heure),
          date(date), capacite(capacite), placesReservees(0), prix(prix) {}

    const string& getNumero() const { return numero; }
    const string& getDepart() const { return depart; }
    const string& getDestination() const { return destination; }
    const Date& getDate() const { return date; }
    double getPrix() const { return prix; }
    int getCapacite() const { return capacite; }
    int placesDisponibles() const { return capacite - placesReservees; }
    bool estComplet() const { return placesReservees >= capacite; }

    void reserverPlace() {
        if (estComplet()) throw runtime_error("Vol complet : plus aucune place disponible.");
        ++placesReservees;
    }
    void libererPlace() {
        if (placesReservees > 0) --placesReservees;
    }

    void afficher() const {
        cout << "  Vol " << numero << " : " << depart << " -> " << destination
             << " | " << date.toString() << " a " << heure
             << " | Prix : " << fixed << setprecision(0) << prix << " FCFA"
             << " | Places libres : " << placesDisponibles() << "/" << capacite << endl;
    }
};

// ------------------------------------------------------------
//  Statuts
// ------------------------------------------------------------
enum class StatutReservation { EN_ATTENTE, CONFIRMEE, ANNULEE };
enum class StatutPaiement { PAYE, REMBOURSE_TOTAL, REMBOURSE_PARTIEL, NON_REMBOURSE };

string versTexte(StatutReservation s) {
    switch (s) {
        case StatutReservation::EN_ATTENTE: return "EN ATTENTE";
        case StatutReservation::CONFIRMEE:  return "CONFIRMEE";
        default:                            return "ANNULEE";
    }
}

string versTexte(StatutPaiement s) {
    switch (s) {
        case StatutPaiement::PAYE:              return "PAYE";
        case StatutPaiement::REMBOURSE_TOTAL:   return "REMBOURSE TOTALEMENT";
        case StatutPaiement::REMBOURSE_PARTIEL: return "REMBOURSE PARTIELLEMENT";
        default:                                return "NON REMBOURSE";
    }
}

// ------------------------------------------------------------
//  Classe Reservation (associe 1 passager et 1 vol)
// ------------------------------------------------------------
class Reservation {
private:
    int id;
    Passager* passager;
    Vol* vol;
    StatutReservation statut;

public:
    Reservation(int id, Passager* passager, Vol* vol)
        : id(id), passager(passager), vol(vol), statut(StatutReservation::EN_ATTENTE) {}

    int getId() const { return id; }
    Passager* getPassager() const { return passager; }
    Vol* getVol() const { return vol; }
    StatutReservation getStatut() const { return statut; }
    bool estAnnulee() const { return statut == StatutReservation::ANNULEE; }

    // Validation de la reservation (apres paiement)
    void confirmer() {
        if (statut != StatutReservation::EN_ATTENTE)
            throw runtime_error("Cette reservation ne peut pas etre validee (statut : " +
                                versTexte(statut) + ").");
        statut = StatutReservation::CONFIRMEE;
    }

    void annuler() {
        if (estAnnulee()) throw runtime_error("Cette reservation est deja annulee.");
        statut = StatutReservation::ANNULEE;
        vol->libererPlace();
    }

    void afficher() const {
        cout << "  Reservation #" << id << " | " << passager->getNomComplet()
             << " | Vol " << vol->getNumero() << " (" << vol->getDepart() << " -> "
             << vol->getDestination() << ", " << vol->getDate().toString() << ")"
             << " | Statut : " << versTexte(statut) << endl;
    }
};

// ------------------------------------------------------------
//  Classe Paiement (lie a une reservation)
// ------------------------------------------------------------
class Paiement {
private:
    int id;
    Reservation* reservation;
    double montant;
    string mode;
    StatutPaiement statut;
    double montantRembourse;

public:
    Paiement(int id, Reservation* reservation, double montant, const string& mode)
        : id(id), reservation(reservation), montant(montant), mode(mode),
          statut(StatutPaiement::PAYE), montantRembourse(0) {}

    int getId() const { return id; }
    Reservation* getReservation() const { return reservation; }
    double getMontant() const { return montant; }
    double getMontantRembourse() const { return montantRembourse; }
    StatutPaiement getStatut() const { return statut; }

    // taux : 1.0 = 100 %, 0.5 = 50 %, 0 = aucun remboursement
    void rembourser(double taux) {
        montantRembourse = montant * taux;
        if (taux >= 1.0)      statut = StatutPaiement::REMBOURSE_TOTAL;
        else if (taux > 0.0)  statut = StatutPaiement::REMBOURSE_PARTIEL;
        else                  statut = StatutPaiement::NON_REMBOURSE;
    }

    void afficher() const {
        cout << "  Paiement #" << id << " | Reservation #" << reservation->getId()
             << " | " << fixed << setprecision(0) << montant << " FCFA (" << mode << ")"
             << " | " << versTexte(statut);
        if (montantRembourse > 0) cout << " (" << montantRembourse << " FCFA rembourses)";
        cout << endl;
    }
};

// ------------------------------------------------------------
//  Classe Agence : gere les collections et les regles metier
// ------------------------------------------------------------
class Agence {
private:
    vector<unique_ptr<Passager>> passagers;
    vector<unique_ptr<Vol>> vols;
    vector<unique_ptr<Reservation>> reservations;
    vector<unique_ptr<Paiement>> paiements;
    int prochainIdPassager = 1, prochainIdReservation = 1, prochainIdPaiement = 1;

    static string minuscules(string s) {
        transform(s.begin(), s.end(), s.begin(),
                  [](unsigned char c) { return static_cast<char>(tolower(c)); });
        return s;
    }

    Paiement* paiementDe(const Reservation* r) const {
        for (const auto& p : paiements)
            if (p->getReservation() == r) return p.get();
        return nullptr;
    }

public:
    // ---- Passagers ----
    Passager* enregistrerPassager(const string& nom, const string& prenom, const string& tel) {
        if (nom.empty() || prenom.empty()) throw invalid_argument("Le nom et le prenom sont obligatoires.");
        passagers.push_back(make_unique<Passager>(nouvelIdPassager(), nom, prenom, tel));
        return passagers.back().get();
    }
    int nouvelIdPassager() { return prochainIdPassager++; }

    Passager* trouverPassager(int id) const {
        for (const auto& p : passagers)
            if (p->getId() == id) return p.get();
        return nullptr;
    }

    // ---- Vols ----
    Vol* ajouterVol(const string& numero, const string& depart, const string& destination,
                    const Date& date, const string& heure, int capacite, double prix) {
        if (numero.empty()) throw invalid_argument("Le numero de vol est obligatoire.");
        if (capacite <= 0) throw invalid_argument("La capacite doit etre positive.");
        if (prix < 0) throw invalid_argument("Le prix ne peut pas etre negatif.");
        if (trouverVol(numero)) throw runtime_error("Un vol portant ce numero existe deja.");
        vols.push_back(make_unique<Vol>(numero, depart, destination, date, heure, capacite, prix));
        return vols.back().get();
    }

    Vol* trouverVol(const string& numero) const {
        for (const auto& v : vols)
            if (minuscules(v->getNumero()) == minuscules(numero)) return v.get();
        return nullptr;
    }

    vector<Vol*> volsDisponibles() const {
        vector<Vol*> resultat;
        for (const auto& v : vols)
            if (!v->estComplet()) resultat.push_back(v.get());
        return resultat;
    }

    // Recherche par numero, ville de depart ou destination (partielle, insensible a la casse)
    vector<Vol*> rechercherVols(const string& critere) const {
        vector<Vol*> resultat;
        string c = minuscules(critere);
        for (const auto& v : vols) {
            if (minuscules(v->getNumero()).find(c) != string::npos ||
                minuscules(v->getDepart()).find(c) != string::npos ||
                minuscules(v->getDestination()).find(c) != string::npos)
                resultat.push_back(v.get());
        }
        return resultat;
    }

    // ---- Reservations ----
    Reservation* reserver(int idPassager, const string& numeroVol) {
        Passager* p = trouverPassager(idPassager);
        if (!p) throw runtime_error("Regle 1 : le passager doit etre enregistre avant de reserver.");
        Vol* v = trouverVol(numeroVol);
        if (!v) throw runtime_error("Vol introuvable.");
        for (const auto& r : reservations)   // Regle 3
            if (r->getPassager() == p && r->getVol() == v && !r->estAnnulee())
                throw runtime_error("Regle 3 : ce passager a deja une reservation sur ce vol.");
        v->reserverPlace();                  // Regle 2 (leve une exception si complet)
        reservations.push_back(make_unique<Reservation>(prochainIdReservation++, p, v));
        return reservations.back().get();
    }

    Reservation* trouverReservation(int id) const {
        for (const auto& r : reservations)
            if (r->getId() == id) return r.get();
        return nullptr;
    }

    // ---- Paiement : valide la reservation ----
    Paiement* enregistrerPaiement(int idReservation, const string& mode) {
        Reservation* r = trouverReservation(idReservation);
        if (!r) throw runtime_error("Reservation introuvable.");
        if (r->estAnnulee())                                  // Regle 5
            throw runtime_error("Regle 5 : une reservation annulee ne peut plus etre payee.");
        if (paiementDe(r)) throw runtime_error("Cette reservation est deja payee.");
        paiements.push_back(make_unique<Paiement>(prochainIdPaiement++, r, r->getVol()->getPrix(), mode));
        r->confirmer();                                       // Regle 6
        return paiements.back().get();
    }

    // ---- Annulation avec politique de remboursement (Regle 7) ----
    // >= 7 jours avant le depart : 100 % | de 3 a 6 jours : 50 % | moins de 3 jours : 0 %
    string annulerReservation(int idReservation) {
        Reservation* r = trouverReservation(idReservation);
        if (!r) throw runtime_error("Reservation introuvable.");
        if (r->estAnnulee()) throw runtime_error("Cette reservation est deja annulee.");

        ostringstream message;
        Paiement* p = paiementDe(r);
        if (p) {
            int jours = r->getVol()->getDate().joursRestants();
            double taux = (jours >= 7) ? 1.0 : (jours >= 3 ? 0.5 : 0.0);
            p->rembourser(taux);
            message << "Reservation annulee. Remboursement : " << fixed << setprecision(0)
                    << p->getMontantRembourse() << " FCFA (" << versTexte(p->getStatut()) << ").";
        } else {
            message << "Reservation annulee (aucun paiement effectue, rien a rembourser).";
        }
        r->annuler();
        return message.str();
    }

    // ---- Affichages ----
    void afficherToutesReservations() const {
        if (reservations.empty()) { cout << "  Aucune reservation.\n"; return; }
        for (const auto& r : reservations) r->afficher();
    }

    void afficherDetailReservation(int id) const {
        Reservation* r = trouverReservation(id);
        if (!r) throw runtime_error("Reservation introuvable.");
        cout << "\n--- Informations de la reservation #" << id << " ---\n";
        r->afficher();
        r->getPassager()->afficher();
        r->getVol()->afficher();
        Paiement* p = paiementDe(r);
        if (p) p->afficher(); else cout << "  Paiement : aucun\n";
    }
};

// ------------------------------------------------------------
//  Fonctions utilitaires de saisie (robustes aux erreurs)
// ------------------------------------------------------------
string lireTexte(const string& message) {
    string s;
    cout << message;
    if (!getline(cin, s)) { cout << "\nFin de saisie.\n"; exit(0); }
    return s;
}

int lireEntier(const string& message) {
    while (true) {
        string s = lireTexte(message);
        try {
            size_t pos;
            int n = stoi(s, &pos);
            if (pos == s.size()) return n;
        } catch (...) {}
        cout << "  Erreur : veuillez saisir un nombre entier.\n";
    }
}

double lireReel(const string& message) {
    while (true) {
        string s = lireTexte(message);
        try {
            size_t pos;
            double x = stod(s, &pos);
            if (pos == s.size()) return x;
        } catch (...) {}
        cout << "  Erreur : veuillez saisir un nombre.\n";
    }
}

Date lireDate(const string& message) {
    while (true) {
        Date d;
        if (Date::lire(lireTexte(message), d)) return d;
        cout << "  Erreur : date invalide (format JJ/MM/AAAA).\n";
    }
}

// ------------------------------------------------------------
//  Menu
// ------------------------------------------------------------
void afficherMenu() {
    cout << "\n=========== AGENCE DE VOYAGE - MENU PRINCIPAL ===========\n"
         << " 1) Enregistrer un passager\n"
         << " 2) Ajouter un vol\n"
         << " 3) Afficher les vols disponibles\n"
         << " 4) Rechercher un vol\n"
         << " 5) Effectuer une reservation\n"
         << " 6) Enregistrer un paiement\n"
         << " 7) Annuler une reservation\n"
         << " 8) Consulter les reservations\n"
         << " 9) Afficher les informations d'une reservation\n"
         << "10) Quitter\n"
         << "==========================================================\n";
}

int main() {
    Agence agence;
    int choix = 0;

    do {
        afficherMenu();
        choix = lireEntier("Votre choix : ");
        try {
            switch (choix) {
                case 1: {
                    string nom = lireTexte("Nom : ");
                    string prenom = lireTexte("Prenom : ");
                    string tel = lireTexte("Telephone : ");
                    Passager* p = agence.enregistrerPassager(nom, prenom, tel);
                    cout << "  Passager enregistre avec l'identifiant #" << p->getId() << ".\n";
                    break;
                }
                case 2: {
                    string numero = lireTexte("Numero du vol : ");
                    string depart = lireTexte("Ville de depart : ");
                    string dest = lireTexte("Destination : ");
                    Date d = lireDate("Date (JJ/MM/AAAA) : ");
                    string heure = lireTexte("Heure (HH:MM) : ");
                    int cap = lireEntier("Capacite (nombre de places) : ");
                    double prix = lireReel("Prix du billet (FCFA) : ");
                    agence.ajouterVol(numero, depart, dest, d, heure, cap, prix);
                    cout << "  Vol ajoute avec succes.\n";
                    break;
                }
                case 3: {
                    auto liste = agence.volsDisponibles();
                    cout << "\n--- Vols disponibles ---\n";
                    if (liste.empty()) cout << "  Aucun vol disponible.\n";
                    for (auto* v : liste) v->afficher();
                    break;
                }
                case 4: {
                    auto liste = agence.rechercherVols(lireTexte("Numero, depart ou destination : "));
                    cout << "\n--- Resultat de la recherche ---\n";
                    if (liste.empty()) cout << "  Aucun vol trouve.\n";
                    for (auto* v : liste) v->afficher();
                    break;
                }
                case 5: {
                    int idP = lireEntier("Identifiant du passager : ");
                    string numero = lireTexte("Numero du vol : ");
                    Reservation* r = agence.reserver(idP, numero);
                    cout << "  Reservation #" << r->getId()
                         << " creee (EN ATTENTE). Elle sera confirmee apres paiement.\n";
                    break;
                }
                case 6: {
                    int idR = lireEntier("Numero de la reservation : ");
                    string mode = lireTexte("Mode de paiement (especes/mobile/carte) : ");
                    Paiement* p = agence.enregistrerPaiement(idR, mode);
                    cout << "  Paiement #" << p->getId() << " enregistre. Reservation CONFIRMEE.\n";
                    break;
                }
                case 7: {
                    int idR = lireEntier("Numero de la reservation a annuler : ");
                    cout << "  " << agence.annulerReservation(idR) << endl;
                    break;
                }
                case 8:
                    cout << "\n--- Liste des reservations ---\n";
                    agence.afficherToutesReservations();
                    break;
                case 9:
                    agence.afficherDetailReservation(lireEntier("Numero de la reservation : "));
                    break;
                case 10:
                    cout << "Au revoir !\n";
                    break;
                default:
                    cout << "  Choix invalide (1 a 10).\n";
            }
        } catch (const exception& e) {
            cout << "  [ERREUR] " << e.what() << endl;
        }
    } while (choix != 10);

    return 0;
}
