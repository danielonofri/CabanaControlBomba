Esta es la documentación técnica definitiva para tu proyecto. Está diseñada para que, si alguien (o tú mismo en un año) abre el código, entienda exactamente por qué cada cable está donde está.

---

# 🚀 Proyecto: Monitor de Bomba de Agua (Cabana)

**Dispositivo:** ESP8266 (ESP-01)

**Versión:** 2.0 (Lógica Inversa & Hardware Pull-up)

## 📋 Descripción General

Sistema de monitoreo remoto vía Wi-Fi que detecta el estado de funcionamiento de una bomba de agua mediante la lectura del terminal negativo del relé. Utiliza un servidor web embebido para mostrar el estado en tiempo real, el tiempo de actividad (*uptime*) y el historial del último ciclo de encendido/apagado.

---

## 🔧 Especificaciones de Hardware

Para garantizar la estabilidad del ESP8266, hemos optimizado el uso de los pines:

### 1. Conexión del Pin Crítico (GPIO2)

El GPIO2 es un pin especial en el ESP8266. Para que el módulo arranque correctamente, este pin **debe detectar un nivel ALTO (3.3V)** al encenderse.

* **Resistencia Pull-up:** Se utiliza una resistencia de **$10\text{ k}\Omega$** conectada entre **VCC (3.3V)** y **GPIO2**.
* **Función:** Esta resistencia "tira" el voltaje hacia arriba, manteniendo el estado en `HIGH` (Bomba Apagada) de forma estable.

### 2. Interfaz con el Relé

El sensor aprovecha el cierre del circuito hacia el negativo (GND) que realiza el controlador del relé.

* **Punto de conexión:** El cable de señal va desde el **GPIO2** directamente al borne o pin que el transistor/integrado conmuta a **GND** para activar la bobina del relé.

---

## 🔌 Esquema de Conexiones (Pinout)


- **ESP‑01 VCC (pin 8)** → Fuente regulada de **3.3 V** estable.  
- **ESP‑01 GND (pin 1)** → Común con el GND del relé y del divisor.  
- **ESP‑01 GPIO0 (pin 3)** → Punto medio del divisor resistivo.  
  - **R1 = 10 kΩ** entre la salida de 5 V del relé (IN o señal de estado) y GPIO0.  
  - **R2 = 20 kΩ** entre GPIO0 y GND.  
  - Esto baja los 5 V a ~3.3 V seguros para el ESP‑01.  
- **ESP‑01 GPIO2 (pin 4)** → Libre (antes iba al LED verde, ahora no se usa).  
- **Relé VCC (5 V)** → Fuente de 5 V independiente.  
- **Relé GND** → Común con GND del ESP‑01.  

---

## Esquema simplificado

```
Relé salida (5V activo) ----[10kΩ]----+----> GPIO0 (ESP-01)
                                      |
                                     [20kΩ]
                                      |
                                     GND

ESP-01 VCC (3.3V) --------------------> Fuente 3.3V
ESP-01 GND ---------------------------> GND común
Relé VCC (5V) ------------------------> Fuente 5V
Relé GND -----------------------------> GND común
```

---

👉 Con esto, el ESP‑01 detecta directamente el estado de la bomba a través de **GPIO0**. El LED ya no está, así que el firmware solo registra y envía las fechas de encendido/apagado.

---

## 💻 Lógica del Software

El programa utiliza **Lógica Inversa**. Esto es fundamental para entender el código:

1. **Estado `LOW` (0V):** El controlador del relé ha cerrado el circuito a tierra. La corriente fluye por la bobina. **Resultado: BOMBA ENCENDIDA.**
2. **Estado `HIGH` (3.3V):** El controlador del relé está abierto. La resistencia de $10\text{ k}\Omega$ domina el pin. **Resultado: BOMBA APAGADA.**

### Funciones Principales:

* **NTP Sync:** Sincroniza la hora exacta mediante servidores de internet para registrar eventos.
* **Detección de Flancos:** El `loop()` compara el estado actual con el anterior para guardar la marca de tiempo exacta (`lastOn` / `lastOff`) solo cuando hay un cambio.
* **Web Server:** Sirve una página HTML con autorefresco cada 10 segundos.

---

## 📂 Estructura de Archivos para GitHub

Para subirlo a Git, organiza tu carpeta así:

```text
/BombaCabana
  ├── BombaCabana.ino    <-- El código principal
  ├── .gitignore         <-- Ignora carpetas de compilación (/build)
  └── README.md          <-- Esta documentación

```

---

## ⚠️ Notas de Seguridad y Mantenimiento

* **Aislamiento:** Asegúrate de que los contactos del relé que manejan los 220V/110V de la bomba estén totalmente aislados de la parte de baja tensión (ESP8266).
* **Ruido Eléctrico:** Si notas que el programa marca encendidos falsos muy rápidos, el cable que va al relé podría estar captando interferencia. En ese caso, se puede agregar un pequeño condensador de $100\text{ nF}$ entre GPIO2 y GND.
* **Red Wi-Fi:** Si cambias el router de la cabana, deberás actualizar las variables `ssid` y `password` en el código.

---

**¿Quieres que te genere el archivo `README.md` formateado en lenguaje Markdown listo para copiar y pegar en tu repositorio de GitHub?**
