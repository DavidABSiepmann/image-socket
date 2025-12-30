pragma Singleton
import QtQuick 2.15

QtObject {
    id: eventMessages

    // Mapeamento de códigos para mensagens em português
    readonly property var messages: ({
        // Servidor
        1000: "Servidor iniciado na porta {port}",
        1001: "Servidor parado",
        1002: "Falha ao iniciar servidor",
        1003: "Porta {port} já está em uso",
        1099: "Erro no servidor: {reason}",

        // Clientes
        2000: "Cliente conectado: {alias}",
        2001: "Cliente desconectado: {alias}",
        2002: "Exibindo: {alias}",
        2003: "Nenhum cliente disponível",

        // FPS
        3000: "FPS configurado: {fps}",
        3001: "Erro: nenhum cliente ativo",
        3002: "Erro ao enviar FPS",

        // Frames
        4000: "Frame recebido",
        4001: "Frame perdido",

        // Genérico
        9999: "Erro desconhecido"
    })

    function getMessage(code, details) {
        var template = messages[code] || "Evento desconhecido: " + code

        // Substituir placeholders {key} com valores de details
        if (details) {
            for (var key in details) {
                var placeholder = "{" + key + "}"
                template = template.replace(placeholder, details[key])
            }
        }

        return template
    }
}
