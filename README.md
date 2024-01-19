# Hashtable Project

Ce projet est une implémentation d'une table de hachage en C. Il fournit une structure de données de type clé-valeur, avec des opérations pour insérer, supprimer et rechercher des éléments.

## Caractéristiques

- Implémentation de table de hachage avec chaînage pour gérer les collisions.
- Utilise l'algorithme de hachage FNV-1a pour générer les indices de hachage.
- Supporte les types de données personnalisés grâce à la structure `value`.

## Utilisation

Pour utiliser cette table de hachage, vous devez d'abord créer une instance de `struct hashtable`. Ensuite, vous pouvez utiliser les fonctions fournies pour interagir avec la table de hachage.

```c
struct hashtable table;
hashtable_create(&table);

// Insérer des éléments
hashtable_set_integer(&table, "clé1", 123);
hashtable_set_boolean(&table, "clé2", true);

// Récupérer des éléments
struct value val = hashtable_get(&table, "clé1");
if (value_is_integer(&val)) {
    int64_t num = value_get_integer(&val);
    printf("La valeur est %ld\n", num);
}

// Supprimer des éléments
hashtable_remove(&table, "clé1");

// Détruire la table de hachage
hashtable_destroy(&table);
```