# DynStack Fork

Ovaj repozitorij je kreiran u sklopu Diplomskog projekta na FER-u u akademskoj godini 25/26. Projekt se bavi korištenjem genetskih algoritama za efikasno slaganje kontejnera u dinamičkom okruženju. Kao inspiraciju je uzeto natjecanje [Dynstak](https://dynstack.adaptop.at/) koje se održava svake godine u sklopu konferencije [GECCO](https://gecco-2025.sigevo.org/HomePage).

## Učenje

Za učenje modela smo napisali svoj simulator, koji je jednostavnija verzija službenog simulatora, u vanila C++. Za samo učenje je korištena biblioteka [ECF](https://github.com/djakobovic/ECF).

Za pokretanje procesa učenja pratite sljedeće korake:
1. Preuzmite biblioteku [ECF](https://github.com/djakobovic/ECF)
2. Preuzmite ovaj repozitoriji
3. Prebacite se u direktoriji `dynstack/starterkits/cpp/`
4. Pokrenite naredbu `cmake -S . -B build`, a zatim `cmake --build build`
5. Proces pokrenite naredbom `./build/buffer_simulator ./src/parameters.txt`

U datoteci _best.txt_ je spremljena najbolja jedinka.

## Vizualizacija

Za vizualizaciju je napisan jednostavan web server (može se pokrenuti putem Docker kontejnera) koji prikazuje stanje službenog simulatora, a za pokretanje najboljeg modela je kreiran wrapper koji je isto dostupan putem Docker kontejnera.

Oba kontejnera se mogu pokrenuti na sljedeći način:
1. Datoteku parameters.txt i najbolju jedinku u obliku datoteke best.txt kopirajte u direktoriji `dynstack/viewer/data`
2. Pokrenite Docker engine na svojem računalu
3. Preuzmite docker-compose datoteku
4. Pokrenite ju putem naredbe `docker-compose up -d`

Za _buildanje_ kontejnera trebate preuzeti cijeli repozitoriji i u docker-compose datoteci otkomentirati linije build, a zakomentirati linije image.

Moguće je pokrenuti i samo kontejner za vizualizaciju te koristiti neko od rješenja iz službenog repozitorija za povezivanje s njim.