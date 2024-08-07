#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "src/tp.h"
#include "src/ASCII.h"
#include "src/ANSI-color-codes.h"
#include "src/lista.h"
#define RESET "\x1b[0m"

#define FACIL 0
#define NORMAL 1
#define DIFICIL 2
#define IMPOSIBLE 3

#define JUGADOR_1 0
#define JUGADOR_2 1

#define MAX_NOMBRES 100

typedef struct {
	int dificultad;
	int rondas_por_jugar;
	int obstaculos;
	int puntaje_para_poder_ganar;
	char *pista_2;
} juego_t;

void seleccionar_dificultad(juego_t *juego)
{
	imprimir_dificultades(3, 3, 50, 1, 5, 2, 60, 2, 7, 1, 70, 3, 8, 0, 80,
			      6);
	printf(HYEL "Ingrese una opción -> " RESET);
	if (scanf("%d", &juego->dificultad) != 1)
		return;
	switch (juego->dificultad) {
	case 1:
		juego->dificultad = FACIL;
		juego->rondas_por_jugar = 4;
		juego->obstaculos = 3;
		juego->puntaje_para_poder_ganar = 50;
		break;
	case 2:
		juego->dificultad = NORMAL;
		juego->rondas_por_jugar = 3;
		juego->obstaculos = 5;
		juego->puntaje_para_poder_ganar = 60;
		break;
	case 3:
		juego->dificultad = DIFICIL;
		juego->rondas_por_jugar = 2;
		juego->obstaculos = 7;
		juego->puntaje_para_poder_ganar = 70;
		break;
	case 4:
		juego->dificultad = IMPOSIBLE;
		juego->rondas_por_jugar = 1;
		juego->obstaculos = 8;
		juego->puntaje_para_poder_ganar = 80;
		break;
	default:
		break;
	}
}

juego_t *juego_crear()
{
	juego_t *juego = malloc(sizeof(juego_t));
	if (!juego)
		return NULL;
	juego->dificultad = 0;
	juego->rondas_por_jugar = 0;
	juego->obstaculos = 0;
	juego->puntaje_para_poder_ganar = 0;
	juego->pista_2 = NULL;
	return juego;
}

void juego_destruir(juego_t *juego)
{
	if (juego->pista_2 != NULL)
		free(juego->pista_2);
	free(juego);
}

char *copiar_nombre_pokemon(const char *nombre)
{
	char *nombre_copia = malloc(strlen(nombre) + 1);
	if (nombre_copia == NULL)
		return NULL;
	strcpy(nombre_copia, nombre);
	return nombre_copia;
}

void free_nombres(char **nombres, int cant_poke_counter)
{
	for (int i = 0; i < cant_poke_counter; i++) {
		free(nombres[i]);
	}
	free(nombres);
}

void extraer_nombres(char *csv_pokemones, char **nombres,
		     int *cant_poke_counter)
{
	char *token;
	*cant_poke_counter = 0;
	token = strtok(csv_pokemones, ",");
	while (token != NULL && *cant_poke_counter < MAX_NOMBRES) {
		nombres[*cant_poke_counter] = copiar_nombre_pokemon(token);
		if (nombres[*cant_poke_counter] == NULL) {
			printf("La asignación de memoria falló.\n");
			free_nombres(nombres, *cant_poke_counter);
			return;
		}
		(*cant_poke_counter)++;
		token = strtok(NULL, ",");
	}
}
void seleccionar_pokemon(TP *tp)
{
	char *pokemones = tp_nombres_disponibles(tp);
	if (pokemones == NULL) {
		printf("Error: No se pudieron obtener los pokemones disponibles.\n");
		return;
	}

	char **nombres = malloc(MAX_NOMBRES * sizeof(char *));
	if (nombres == NULL) {
		printf("Error: No se pudo reservar memoria.\n");
		free(pokemones);
		return;
	}

	int cant_poke_counter = 0;
	extraer_nombres(pokemones, nombres, &cant_poke_counter);
	int i = 0;
	while (i < cant_poke_counter) {
		for (int i = 0; i < cant_poke_counter; i++) {
			const struct pokemon_info *poke =
				tp_buscar_pokemon(tp, nombres[i]);
			if (poke == NULL) {
				printf("Error: No se pudo obtener la información del pokemon.\n");
				free_nombres(nombres, cant_poke_counter);
				free(pokemones);
				return;
			}
			imprimir_poke_tabla(i + 1, nombres[i], poke->fuerza,
					    poke->inteligencia, poke->destreza);
			if (i == cant_poke_counter - 1) {
				printf(HYEL
				       "(1) Seleccionar (3) Anterior\n" RESET);
			} else if (i == 0) {
				printf(HYEL
				       "(1) Seleccionar (2) Siguiente\n" RESET);
			} else {
				printf(HYEL
				       "(1) Seleccionar (2) Siguiente (3) Anterior\n" RESET);
			}
			printf(HYEL "Ingrese una opción ->" RESET);
			int rsta;
			if (scanf("%d", &rsta) != 1) {
				printf("Error: Entrada inválida.\n");
				continue;
			}
			if (rsta == 1) {
				char *selected_pokemon =
					copiar_nombre_pokemon(nombres[i]);
				if (selected_pokemon == NULL) {
					printf("Error: No se pudo copiar el nombre del pokemon.\n");
					free_nombres(nombres,
						     cant_poke_counter);
					free(pokemones);
					return;
				}
				tp_seleccionar_pokemon(tp, JUGADOR_1,
						       selected_pokemon);

				free_nombres(nombres, cant_poke_counter);
				free(pokemones);
				free(selected_pokemon);
				return;
			} else if (rsta == 3) {
				if (i > 0) {
					i -= 2;
				} else {
					printf("Ya estás en el primer pokemon.\n");
				}
			}
		}

		free_nombres(nombres, cant_poke_counter);
		free(pokemones);
		return;
	}
}
void crear_pista_al_azar(TP *tp, int i)
{
	int obstaculo = rand() % 3;
	tp_agregar_obstaculo(tp, JUGADOR_2, obstaculo, (unsigned)i);
}

void crear_pista(TP *tp, juego_t *juego, int i)
{
	imprimir_opciones_obstaculos(i + 1);
	printf(HYEL "Ingrese una opción -> " RESET);
	int obstaculo;
	if (scanf("%d", &obstaculo) != 1)
		return;
	tp_agregar_obstaculo(tp, JUGADOR_1, obstaculo - 1, (unsigned)i);
}
char *obtener_al_azal(char **nombres, int count)
{
	int random = rand() % count;
	return nombres[random];
}

void seleccionar_pokemon_al_azar(TP *tp)
{
	char *nombre = tp_nombres_disponibles(tp);
	char **nombres = malloc(MAX_NOMBRES * sizeof(char *));
	if (nombres == NULL) {
		printf("La asignación de memoria falló.\n");
		free(nombre);
		return;
	}
	int cant_poke_counter;

	extraer_nombres(nombre, nombres, &cant_poke_counter);
	char *selected_pokemon = copiar_nombre_pokemon(
		obtener_al_azal(nombres, cant_poke_counter));
	tp_seleccionar_pokemon(tp, JUGADOR_2, selected_pokemon);
	free(nombre);
	free_nombres(nombres, cant_poke_counter);
	free(selected_pokemon);
	return;
}

void tp_jugar_ronda(TP *tp, int ronda, juego_t *juego)
{
	if (ronda == 0) {
		seleccionar_pokemon(tp);
	}
	if (ronda == 0) {
		seleccionar_pokemon_al_azar(tp);
		const struct pokemon_info *poke1 =
			tp_pokemon_seleccionado(tp, JUGADOR_1);
		if (poke1 == NULL) {
			printf("Error: No se pudo obtener la información del pokemon.\n");
			return;
		}
		const struct pokemon_info *poke2 =
			tp_pokemon_seleccionado(tp, JUGADOR_2);
		if (poke2 == NULL) {
			printf("Error: No se pudo obtener la información del pokemon.\n");
			return;
		}
		versus(poke1->nombre, poke2->nombre, poke1->fuerza,
		       poke2->fuerza, poke1->destreza, poke2->destreza,
		       poke1->inteligencia, poke2->inteligencia);
	}
	printf(HYEL "Ingrese 1 para continuar -> " RESET);
	int opt;
	if (scanf("%d", &opt) != 1)
		return;
	if (opt != 1) {
		return;
	}

	if (ronda == 0) {
		for (int i = 0; i < juego->obstaculos; i++) {
			crear_pista(tp, juego, i);
		}
	} else {
		tp_limpiar_pista(tp, JUGADOR_1);
		for (int i = 0; i < juego->obstaculos; i++) {
			crear_pista(tp, juego, i);
		}
	}

	if (ronda == 0) {
		for (int i = 0; i < juego->obstaculos; i++) {
			crear_pista_al_azar(tp, i);
		}
	}
	char *pista1 = tp_obstaculos_pista(tp, JUGADOR_1);
	if (pista1 == NULL) {
		printf("Error: No se pudo obtener la pista.\n");
		return;
	}
	pistas(juego->dificultad, pista1);

	printf(HRED
	       "				            <----- TU PISTA ----->\n" RESET);
	printf(HYEL "Ingrese 1 para continuar -> " RESET);
	if (scanf("%d", &opt) != 1)
		return;
	if (opt != 1) {
		return;
	}
	free(pista1);

	if (ronda == 0) {
		char *pista2 = tp_obstaculos_pista(tp, JUGADOR_2);
		if (pista2 == NULL) {
			printf("Error: No se pudo obtener la pista.\n");
			return;
		}
		int posicion_X = rand() % juego->obstaculos;
		switch (juego->dificultad) {
		case FACIL:
			pista2[posicion_X] = 'X';
			break;
		case NORMAL:
			for (int i = 0; i < juego->obstaculos; i++) {
				pista2[rand() % juego->obstaculos] = 'X';
			}
			break;
		case DIFICIL:
			for (int i = 0; i < juego->obstaculos; i++) {
				pista2[rand() % juego->obstaculos] = 'X';
			}
			break;
		case IMPOSIBLE:
			for (int i = 0; i < juego->obstaculos; i++) {
				pista2[rand() % juego->obstaculos] = 'X';
			}
			break;
		}
		pista2[juego->obstaculos] = '\0';
		juego->pista_2 = copiar_nombre_pokemon(pista2);
		free(pista2);
	}

	pistas(juego->dificultad, juego->pista_2);
	printf(HRED
	       "					<----- PISTA DE TU CONTRINCANTE ----->\n" RESET);
	printf(HYEL "Ingrese 1 para continuar -> " RESET);
	if (scanf("%d", &opt) != 1)
		return;
	if (opt != 1) {
		return;
	}

	unsigned tiempo1 = tp_calcular_tiempo_pista(tp, JUGADOR_1);
	unsigned tiempo2 = tp_calcular_tiempo_pista(tp, JUGADOR_2);

	int resta = (int)(tiempo1 - tiempo2);
	if (resta < 0) {
		resta = -resta;
	}
	float puntaje =
		(float)100 - (float)(100 * resta / (int)(tiempo1 + tiempo2));
	printf("Puntaje: %.2f \n", puntaje);
	if (puntaje >= juego->puntaje_para_poder_ganar) {
		fin_de_la_partida(true, juego->rondas_por_jugar - ronda,
				  puntaje, (int)tiempo1, (int)tiempo2);
	} else {
		fin_de_la_partida(false, juego->rondas_por_jugar - ronda,
				  puntaje, (int)tiempo1, (int)tiempo2);
	}
	if (ronda == juego->rondas_por_jugar - 1) {
		printf(HYEL "Ingrese 1 para continuar -> " RESET);
		if (scanf("%d", &opt) != 1)
			return;
		if (opt != 1) {
			return;
		}

		return;
	}
	printf(HYEL "Desea seguir jugando? (1) si (2) no ->" RESET);
	int seguir;
	if (scanf("%d", &seguir) != 1)
		return;
	if (seguir == 2) {
		juego->rondas_por_jugar = 0;
		return;
	}
}

void tp_jugar(TP *tp, juego_t *juego)
{
	int ronda = 0;
	while (ronda < juego->rondas_por_jugar) {
		tp_jugar_ronda(tp, ronda, juego);
		ronda++;
	}
}

int main(int argc, char const *argv[])
{
	srand((unsigned int)time(NULL));
	juego_t *juego = juego_crear();
	if (!juego) {
		printf("Error: No se pudo crear el juego.\n");
		return 1;
	}

	TP *tp = tp_crear("ejemplo/pokemones.txt");
	if (!tp) {
		printf("Error: No se pudo crear TP.\n");
		juego_destruir(juego);
		return 1;
	}
	imprimir_menu_principal();
	printf(HYEL "Ingrese una opción -> " RESET);
	int opcion;
	if (scanf("%d", &opcion) != 1)
		return 1;
	if (opcion == 1) {
		seleccionar_dificultad(juego);
		tp_jugar(tp, juego);
	}
	juego_destruir(juego);
	tp_destruir(tp);
	salida();
	return 0;
}
