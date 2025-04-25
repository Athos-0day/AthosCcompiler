# Mini C Compiler

Un petit compilateur en Python qui traite un sous-ensemble minimal du langage C.

## Fonctionnalités supportées

Actuellement, ce compilateur peut :

- Lire un fichier source `.c`
- L'analyser lexicalement et syntaxiquement
- Générer du code assembleur (x86-64 / ARM selon l'architecture)
- Compiler le code assembleur en un exécutable avec `gcc`

## Syntaxe C supportée

Le compilateur prend en charge **un seul type de programme** :

```c
int main(void) {
    return 42;
}
