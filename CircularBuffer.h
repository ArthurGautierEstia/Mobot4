#pragma once
#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <array>
#include <QMutex>
#include <QMutexLocker>

/**
 * @brief Buffer circulaire thread-safe pour stocker les frames
 *
 * Template générique, par défaut 100 frames.
 * Écrase automatiquement les anciennes données si le buffer est plein.
 */
template<typename T, size_t Size = 100>
class CircularBuffer {
public:
    CircularBuffer() : m_head(0), m_tail(0), m_count(0) {}

    /**
     * @brief Ajoute un élément au buffer
     * Si le buffer est plein, écrase l'élément le plus ancien
     */
    void push(const T& item) {
        QMutexLocker locker(&m_mutex);

        m_buffer[m_head] = item;
        m_head = (m_head + 1) % Size;

        if (m_count < Size) {
            m_count++;
        }
        else {
            // Buffer plein, on avance la queue
            m_tail = (m_tail + 1) % Size;
        }
    }

    /**
     * @brief Retire et retourne l'élément le plus ancien
     * @return true si succès, false si buffer vide
     */
    bool pop(T& item) {
        QMutexLocker locker(&m_mutex);

        if (m_count == 0) {
            return false;
        }

        item = m_buffer[m_tail];
        m_tail = (m_tail + 1) % Size;
        m_count--;

        return true;
    }

    /**
     * @brief Consulte l'élément le plus ancien sans le retirer
     */
    bool peek(T& item) const {
        QMutexLocker locker(&m_mutex);

        if (m_count == 0) {
            return false;
        }

        item = m_buffer[m_tail];
        return true;
    }

    /**
     * @brief Nombre d'éléments dans le buffer
     */
    size_t size() const {
        QMutexLocker locker(&m_mutex);
        return m_count;
    }

    /**
     * @brief Capacité maximale du buffer
     */
    size_t capacity() const {
        return Size;
    }

    /**
     * @brief Vérifie si le buffer est vide
     */
    bool isEmpty() const {
        QMutexLocker locker(&m_mutex);
        return m_count == 0;
    }

    /**
     * @brief Vérifie si le buffer est plein
     */
    bool isFull() const {
        QMutexLocker locker(&m_mutex);
        return m_count == Size;
    }

    /**
     * @brief Vide le buffer
     */
    void clear() {
        QMutexLocker locker(&m_mutex);
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

private:
    std::array<T, Size> m_buffer;
    size_t m_head;
    size_t m_tail;
    size_t m_count;
    mutable QMutex m_mutex;
};

#endif // CIRCULARBUFFER_H
