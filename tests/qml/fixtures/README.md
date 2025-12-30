# QML Test Fixtures

Fixtures reutilizáveis para suportar testes de componentes QML sem dependências de backend real.

## 📋 Visão Geral

| Fixture | Propósito | Uso |
|---------|-----------|-----|
| **MockBackend.qml** | Simular C++ backend (ImageServerBridge) | Injetar estado em testes |
| **TestRoot.qml** | Item raiz controlado para testes | Hospedar componentes testados |
| **ComponentLoader.qml** | Carregar componentes dinamicamente | Testar carregamento de componentes |
| **SignalRecorder.qml** | Gravar emissões de signals | Verificar signals em testes |
| **AnimationHelper.qml** | Detectar estado de animações | Testar transições e animações |

---

## 🔧 MockBackend.qml

**Propósito:** Simula o backend C++ (ImageServerBridge) para testes QML.

### Propriedades

```qml
MockBackend {
    serverState: "Idle"              // "Idle", "Running", "Error"
    connectionState: "Disconnected"  // "Disconnected", "Connected", "Connecting"
    statusMessage: ""
    currentFps: 0
    configuredFps: 30
    activeClientAlias: ""
    clientCount: 0
    clientList: []                   // [{alias, id}, ...]
}
```

### Signals

```qml
serverStateChanged(string newState)
connectionStateChanged(string newState)
statusMessageChanged(string message)
fpsChanged(int fps)
configuredFpsChanged(int fps)
activeClientChanged(string alias)
clientCountChanged(int count)
errorOccurred(string errorMessage)
```

### Métodos

```qml
startServer()                       // Muda estado para "Running"
stopServer()                        // Reseta para "Idle"
setFps(value)                       // Atualiza FPS
simulateError(message)              // Simula erro
simulateClientConnect(alias)        // Simula cliente conectado
simulateClientDisconnect()          // Simula cliente desconectado
reset()                             // Limpa todo estado
```

---

## 🏗️ TestRoot.qml

**Propósito:** Fornecer item raiz controlado para testes.

### Propriedades

```qml
TestRoot {
    width: 800              // Ajustável
    height: 600             // Ajustável
    color: "transparent"    // Sem styling
    
    // Adicionar componente para testar:
    YourComponent {
        anchors.fill: parent
    }
}
```

---

## 📦 ComponentLoader.qml

**Propósito:** Carregar componentes dinâmicamente e capturar erros.

### Propriedades

```qml
ComponentLoader {
    id: loader
    source: "path/to/Component.qml"
    
    // Status strings:
    readonly property string statusString  // "Null", "Ready", "Loading", "Error"
    readonly property bool isLoaded
    readonly property bool hasError
    readonly property string errorString
}
```

---

## 🎙️ SignalRecorder.qml

**Propósito:** Gravar emissões de signals em testes.

### Propriedades

```qml
SignalRecorder {
    id: recorder
    target: myObject
    signalName: "clicked"
    
    readonly property int count         // Número de emissões
    readonly property var lastArgs      // Argumentos da última emissão
    readonly property var history       // Histórico de emissões
    property bool recordHistory: false  // Registrar histórico
}
```

### Métodos

```qml
recordSignal(args)              // Gravar emissão com argumentos
recordWithArgs(arg0, arg1, ...) // Gravar com argumentos específicos
clear()                         // Resetar contadores
emissionCount()                 // Obter contagem
wasEmitted()                    // Verificar se foi emitido
emittedExactly(n)              // Verificar contagem exata
lastArgument(index)            // Obter argumento específico
```

---

## ✨ AnimationHelper.qml

**Propósito:** Detectar estado de animações para testes.

### Propriedades

```qml
AnimationHelper {
    id: animHelper
    target: toastItem
    property: "opacity"
    
    readonly property bool isAnimating    // Animação em progresso?
    readonly property var currentValue    // Valor atual
    readonly property var startValue      // Valor inicial
    readonly property var endValue        // Valor final
    readonly property int duration        // Duração (ms)
    readonly property int elapsed         // Tempo decorrido (ms)
}
```

### Métodos

```qml
startMonitoring()   // Começar a monitorar
stopMonitoring()    // Parar de monitorar
reset()             // Resetar estado
isComplete()        // Animação terminou?
```

---

## 🚫 Restrições

- ❌ Fixtures NÃO contêm assertions
- ❌ Fixtures NÃO contêm lógica de negócio
- ❌ Fixtures NÃO acessam filesystem, network, ou threads
- ❌ Fixtures NÃO fazem validação de entrada
- ✅ Fixtures são puro estado + signal emission

---

## 📖 Referências

- Qt Quick Test: https://doc.qt.io/qt-5/qtquicktest-index.html
- QML Testing: https://doc.qt.io/qt-5/qml-qttest.html
- Signal/Slot: https://doc.qt.io/qt-5/moc.html

