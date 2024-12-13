package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net"
	"net/http"
	"sync"
	"time"
)

func main() {
	wg := new(sync.WaitGroup)
	now := time.Now()

	for i := 0; i < 1; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			// id := fmt.Sprint(i)
			// checkReadiness(id)
			GOTCP()
		}(i)
	}
	wg.Wait()
	fmt.Printf("\nTTC: %v\n\n", time.Since(now))
}

func checkReadiness(id string) {
	const url = "http://0.0.0.0:1234"
	client := &http.Client{}
	fmt.Println(" START REQUEST ID:", id)

	req, err := http.NewRequest(http.MethodGet, url, nil)
	if err != nil {
		panic(err)
	}

	// req.Header.Set("X-REQUEST-ID", id)

	fmt.Println(req)
	resp, err := client.Do(req)
	if err != nil {
		fmt.Println(" ERROR REQUESTID:", id, err.Error())
	} else {
		fmt.Println(" DONE REQUEST ID;", id, http.StatusText(resp.StatusCode))
	}

}

func GOTCP() {
	serverAddr := "127.0.0.1:1234"

	conn, err := net.Dial("tcp", serverAddr)
	if err != nil {
		fmt.Println("Connection failed:", err)
		return
	}
	defer conn.Close()

	// Pesan yang akan dikirim
	message := "hello"

	// Buat buffer untuk mengirim header + payload
	var buffer bytes.Buffer
	messageLen := uint32(len(message))

	// Tulis panjang pesan ke buffer
	err = binary.Write(&buffer, binary.BigEndian, messageLen)
	if err != nil {
		fmt.Println("Error writing message length:", err)
		return
	}

	// Tulis pesan ke buffer
	buffer.WriteString(message)

	// Kirim data ke server
	_, err = conn.Write(buffer.Bytes())
	if err != nil {
		fmt.Println("Error sending data:", err)
		return
	}
	fmt.Println("Message sent:", message)

	// Terima respons dari server
	replyHeader := make([]byte, 4)
	_, err = conn.Read(replyHeader)
	if err != nil {
		fmt.Println("Error reading response header:", err)
		return
	}

	// Baca panjang respons
	var replyLen uint32
	err = binary.Read(bytes.NewReader(replyHeader), binary.BigEndian, &replyLen)
	if err != nil {
		fmt.Println("Error reading response length:", err)
		return
	}

	if replyLen > 0 {
		replyBody := make([]byte, replyLen)
		_, err = conn.Read(replyBody)
		if err != nil {
			fmt.Println("Error reading response body:", err)
			return
		}

		fmt.Println("Server response:", string(replyBody))
	}
}
