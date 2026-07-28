# LifeTrack
**Progetto di programmazione a oggetti per un'applicazione desktop di gestione di attività, impegni e scadenze.**

---

## Informazioni sul Progetto
**LifeTrack** è un'applicazione desktop sviluppata in C++ utilizzando il **framework Qt**. È progettata per aiutare gli utenti a organizzare, monitorare e gestire in modo efficiente vari aspetti della vita quotidiana, tra cui impegni personali, scadenze universitarie, turni di lavoro, promemoria, compleanni e note.

Il progetto segue rigorosamente i principi della **Programmazione Orientata agli Oggetti (OOP)**, facendo ampio uso di gerarchie polimorfiche, pattern architetturali (come il pattern **Visitor** per la gestione della persistenza) e una struttura del codice pulita e modulare.

---

## Caratteristiche Principali
*   **Gestione di diverse tipologie di attività:** Supporta categorie specializzate di eventi e task:
    *   *Impegni* (Attività e appuntamenti generali)
    *   *Scadenze* (Task con livelli di priorità e categorie)
    *   *Università* (Lezioni, laboratori e ricevimenti con docenti e aule)
    *   *Lavoro* (Turni, riunioni aziendali e consegne di report)
    *   *Promemoria* (Note e liste della spesa o di debug)
    *   *Compleanni* (Promemoria per date importanti con idee regalo e legami)
*   **Persistenza dei Dati:** Esportazione e importazione dei dati tramite formati standard **JSON** e **XML**.
*   **Pattern Visitor:** Implementazione del design pattern Visitor per gestire la logica di salvataggio e serializzazione in modo pulito e indipendente per ciascun tipo di attività, senza violare l'incapsulamento.
*   **Interfaccia Grafica Intuitiva:** Sviluppata con Qt Widgets e interfacce utente personalizzate (`.ui`) per offrire un'esperienza desktop fluida e gradevole.

---

## Tecnologie e Architettura
*   **Linguaggio:** C++ (C++17)
*   **Framework:** Qt (Moduli Core, GUI, Widgets, XML)
*   **Build System:** qmake (`.pro`)
*   **Design Patterns:** Visitor Pattern (`AttivitaVisitor`, `JsonSaveVisitor`, `XmlSaveVisitor`)

---

## Struttura del Progetto
```text
PAO_MARINI_BELLUZ/
│
├── attivita/          # Classi delle attività (gerarchia polimorfica) e adapter
├── model/             # Modelli e strutture dati dell'applicazione
├── persistenza/       # Logica di persistenza (Visitor JSON e XML)
├── view/              # Controller della GUI e logica della finestra principale
├── forms/             # File di interfaccia grafica Qt (.ui) e immagini collegate
├── assets.qrc         # File di risorse Qt per icone e immagini
├── PAO.pro            # File di configurazione del progetto Qt
└── main.cpp           # Entry point dell'applicazione
