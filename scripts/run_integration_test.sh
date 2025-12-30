#!/usr/bin/env bash
set -eu

# Script de integração para compilar, iniciar servidor e cliente com logs.
# Uso: ./scripts/run_integration_test.sh

# --- Configurações (placeholders; preencha antes de executar) ---
# Root é resolvido relativo ao script para evitar caminhos com usuário
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
SERVER_BIN="$BUILD_DIR/apps/server/server-receiver"
CLIENT_BIN="$BUILD_DIR/apps/client/send_image_client"
# Ajuste estes caminhos para seus arquivos de vídeo locais antes de executar
# VIDEO_PATH1 é OBRIGATÓRIO e deve existir
VIDEO_PATH1="/path/to/video1.mp4"
# VIDEO_PATH2 e VIDEO_PATH3 são opcionais: se permanecerem como placeholder ou não existirem, estes clientes serão ignorados
VIDEO_PATH2="/path/to/video2.mp4"
VIDEO_PATH3="/path/to/video3.mp4"
SERVER_HOST="127.0.0.1"
SERVER_PORT="5021"

LOG_DIR="/tmp"
SERVER_LOG="$LOG_DIR/imagesocket_server.log"
CLIENT_LOG="$LOG_DIR/imagesocket_client.log"

# Timeouts
START_TIMEOUT=10
CONNECT_TIMEOUT=10

# Helper: print and flush
info() { echo "[INFO] $*"; }
err() { echo "[ERROR] $*" >&2; }

# 1) Compilar
info "Compilando projeto..."
if ! /usr/bin/cmake --build "$BUILD_DIR" -j 16; then
    err "Falha na compilação. Logs mostrados acima. Abortando."
    exit 1
fi
info "Compilação concluída com sucesso."

# 2) Verificar binários
if [[ ! -x "$SERVER_BIN" ]]; then
    err "Servidor não encontrado ou não executável: $SERVER_BIN"
    exit 1
fi
if [[ ! -x "$CLIENT_BIN" ]]; then
    err "Cliente não encontrado ou não executável: $CLIENT_BIN"
    exit 1
fi

# Cleanup de runs anteriores (seguro)
info "Limpando processos antigos (se houver)..."
pkill -f "$SERVER_BIN" || true
pkill -f "$CLIENT_BIN" || true
sleep 0.3

# 3) Iniciar servidor em background e salvar logs
info "Iniciando servidor em background (logs: $SERVER_LOG)..."
: > "$SERVER_LOG"
"$SERVER_BIN" --host "$SERVER_HOST" --port "$SERVER_PORT" > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!
info "PID do servidor: $SERVER_PID"

# Esperar até que o servidor esteja pronto (grep no log)
info "Aguardando inicialização do servidor (timeout ${START_TIMEOUT}s)..."
READY=false
for i in $(seq 1 $START_TIMEOUT); do
    if grep -q "WebSocketServer started on port" "$SERVER_LOG"; then
        READY=true
        break
    fi
    if ! kill -0 "$SERVER_PID" 2>/dev/null; then
        err "Servidor finalizou inesperadamente. Cheque $SERVER_LOG"
        exit 1
    fi
    sleep 1
done

if ! $READY ; then
    err "Servidor não reportou inicialização após ${START_TIMEOUT}s. Cheque $SERVER_LOG"
    kill "$SERVER_PID" 2>/dev/null || true
    exit 1
fi
info "Servidor inicializado com sucesso."

# 4) Validar paths de vídeo e iniciar cliente(s)
# Helper para detectar placeholders comuns
is_placeholder() {
    case "$1" in
        */path/to/*|*<*|*VIDEO_PATH*) return 0 ;;
        *) return 1 ;;
    esac
}

# VIDEO_PATH1 é obrigatório
if is_placeholder "$VIDEO_PATH1" || [[ ! -f "$VIDEO_PATH1" ]]; then
    err "VIDEO_PATH1 não definido corretamente ou arquivo não existe: $VIDEO_PATH1"
    err "Edite o script e defina VIDEO_PATH1 para o caminho completo do arquivo de vídeo."
    exit 1
fi

# VIDEO_PATH2 e VIDEO_PATH3 são opcionais - pular se placeholder/ausente
START_BOB=true
START_CHARLIE=true
if is_placeholder "$VIDEO_PATH2" || [[ ! -f "$VIDEO_PATH2" ]]; then
    info "VIDEO_PATH2 ausente ou placeholder ($VIDEO_PATH2). Pulando cliente 'Bob'."
    START_BOB=false
fi
if is_placeholder "$VIDEO_PATH3" || [[ ! -f "$VIDEO_PATH3" ]]; then
    info "VIDEO_PATH3 ausente ou placeholder ($VIDEO_PATH3). Pulando cliente 'Charlie'."
    START_CHARLIE=false
fi

# Iniciar cliente Alice (obrigatório)
info "Iniciando cliente (alias: Alice) em background (logs: $CLIENT_LOG)..."
: > "$CLIENT_LOG"
"$CLIENT_BIN" --host "$SERVER_HOST" --port "$SERVER_PORT" --video "$VIDEO_PATH1" --alias Alice > "$CLIENT_LOG" 2>&1 &
CLIENT_ALICE_PID=$!
info "PID do cliente Alice: $CLIENT_ALICE_PID"

# 5) Validar conexão cliente no servidor (procurar 'Accepted new WebSocket connection' no log)
info "Aguardando conexão do cliente (timeout ${CONNECT_TIMEOUT}s)..."
CONNECTED=false
for i in $(seq 1 $CONNECT_TIMEOUT); do
    if grep -q "Accepted new WebSocket connection" "$SERVER_LOG" || grep -q "ClientConnected" "$SERVER_LOG" ; then
        CONNECTED=true
        break
    fi
    if ! kill -0 "$CLIENT_ALICE_PID" 2>/dev/null; then
        err "Cliente finalizou inesperadamente. Cheque $CLIENT_LOG"
        break
    fi
    sleep 1
done

if $CONNECTED ; then
    info "Cliente conectado com sucesso."
else
    err "Cliente não conectou dentro do timeout. Verifique logs: $SERVER_LOG $CLIENT_LOG"
fi

# 6) Conectar mais clientes conforme desejado (exemplo com Bob)
if $START_BOB ; then
    info "Iniciando cliente (alias: Bob) em background (logs: $CLIENT_LOG)..."
    : > "$CLIENT_LOG"
    "$CLIENT_BIN" --host "$SERVER_HOST" --port "$SERVER_PORT" --video "$VIDEO_PATH2" --alias Bob > "$CLIENT_LOG" 2>&1 &
    CLIENT_PID_BOB=$!
    info "PID do cliente Bob: $CLIENT_PID_BOB"
else
    CLIENT_PID_BOB=""
fi

# Conectar mais um cliente (Charlie)
if $START_CHARLIE ; then
    info "Iniciando cliente (alias: Charlie) em background (logs: $CLIENT_LOG)..."
    : > "$CLIENT_LOG"
    "$CLIENT_BIN" --host "$SERVER_HOST" --port "$SERVER_PORT" --video "$VIDEO_PATH3" --alias Charlie > "$CLIENT_LOG" 2>&1 &
    CLIENT_PID_CHARLIE=$!
    info "PID do cliente Charlie: $CLIENT_PID_CHARLIE"
else
    CLIENT_PID_CHARLIE=""
fi

# 6) Resumo e instruções de limpeza
info "Resumo:"
info "  Server PID: $SERVER_PID (logs: $SERVER_LOG)"
info "  Alice PID: ${CLIENT_ALICE_PID:-N/A}"
info "  Bob PID: ${CLIENT_PID_BOB:-N/A}"
info "  Charlie PID: ${CLIENT_PID_CHARLIE:-N/A}"
info "Para encerrar: kill $SERVER_PID ${CLIENT_ALICE_PID:-} ${CLIENT_PID_BOB:-} ${CLIENT_PID_CHARLIE:-}"

# Trap para matar background quando o script terminar (não mata se o usuário quiser manter rodando)
cleanup() {
    info "Limpando processos de teste..."
    kill "${CLIENT_ALICE_PID:-}" 2>/dev/null || true
    kill "${CLIENT_PID_BOB:-}" 2>/dev/null || true
    kill "${CLIENT_PID_CHARLIE:-}" 2>/dev/null || true
    kill "$SERVER_PID" 2>/dev/null || true
}

# Não executar cleanup automaticamente para deixar servidor/cliente rodando para inspeção
# trap cleanup EXIT

exit 0
