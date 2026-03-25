# RetroMemo 🎮

¡Bienvenido a **RetroMemo**! Un juego de memoria clásico desarrollado enteramente en lenguaje C utilizando la librería gráfica **SDL2**.

Este proyecto ha sido refactorizado y limpiado especialmente para formar parte de mi portafolio personal, demostrando mis habilidades en manejo de memoria manual, estructuras dinámicas y lógicas de máquina de estados.

## 🚀 Requisitos y Cómo Jugar

¡La idea es que puedas jugar sin volverte loco compilando todo localmente!

1. Ve a la pestaña de [**Releases**](../../releases) en este repositorio.
2. Descarga el archivo `.zip` más reciente (ej. `RetroMemo_Release.zip`).
3. Extrae el contenido completo en una carpeta de tu computadora.
4. Haz doble clic en `RetroMemo.exe` y ¡a jugar!

> **Nota para usuarios de Windows:** El ejecutable requiere las librerías dinámicas de SDL2. Si al intentar correr el juego obtienes errores de `.dll` faltantes (como `SDL2.dll`, `SDL2_image.dll`, etc.), asegúrate de bajar el Release que dice "Standalone" o que incluya los DLLs directamente en la carpeta.

## 🕹️ Características del Juego

- **Múltiples Dificultades:** Juega en tableros de 3x4 (Bajo), 4x4 (Medio) o 4x5 (Alto).
- **Modo Singleplayer y Multiplayer:** Desafía a un amigo en partidas por turnos o intenta superar tu récord en solitario.
- **Sistema de Puntuación Persistent:** Ranking local guardado automáticamente con el Top 5 mejores jugadores.
- **Temáticas de Cartas:** Elige entre el estilo "Medieval" o "Griego".
- **Audio Interactivo:** Efectos de sonido para volteos, matches y música ambiental durante el juego.

## 💻 Aspectos Técnicos (Para Desarrolladores)

Si querés explorar cómo está construido el juego por dentro:

- **Lenguaje C puro**: Orientado fuertemente a estructuración manual, gestión responsable de punteros y asignación dinámica.
- **TDA Genérico propio**: Para la organización modular construí un Tipo de Dato Abstracto `tVector` al estilo orientativo de C++ que aloja dinámicamente cartas, jugadores y etiquetas gráficas sin desbordamientos de buffer.
- **Gestión de Interfaz basada en Pilas**: Para no perder de vista por dónde navegaba el jugador entre menúes y el board del juego, apilo el historial en una máquina de estados controlada para el render.
- **Ecosistema SDL2 Completo**: Aprovecha las bondades base multihilos para texturas con `SDL2_image`, renderizado de fuentes true-type con `SDL2_ttf` y carga asincrónica de mixeos en formato WAV vía `SDL2_mixer`.

### Compilar desde el código fuente

Si quieres recrear el binario (asumiendo entorno GCC/MinGW con SDL2 en tu PATH):

```bash
# Ejemplo de línea de comandos para compilación
gcc *.c -o RetroMemo.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
```
