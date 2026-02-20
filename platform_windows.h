// ============================================================================
// platform_windows.h - Centralise les includes Windows + gestion des conflits
// ============================================================================
// Objectif :
//   - Empêcher windows.h d'inclure l'ancien <winsock.h>
//   - Prévenir les macros min/max problématiques
//   - Garantir que <winsock2.h> est toujours inclus avant windows.h
//   - Permettre d'inclure Windows dans N'IMPORTE QUEL ORDRE ailleurs
//
// À inclure dans les .h ou .cpp qui utilisent des API Windows ou Winsock
// ============================================================================

#pragma once

// Empêcher windows.h d'inclure <winsock.h> (qui entre en conflit avec <winsock2.h>)
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

// Supprimer les macros min/max de windows.h
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Limiter windows.h au strict nécessaire
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Inclure les headers Windows dans le bon ordre
#include <winsock2.h>   // API réseau moderne (DOIT précéder windows.h)
#include <ws2tcpip.h>   // Fonctions réseau étendues (inet_pton, etc.)
#include <windows.h>    // WinAPI principale

// ============================================================================
// Optionnel : libs Winsock à linker automatiquement (si pas en CMake ou autre)
// #pragma comment(lib, "Ws2_32.lib")
// ============================================================================
